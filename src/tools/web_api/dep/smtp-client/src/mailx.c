/**
 * @file
 * @brief POSIX mailx utility.
 * @author James Humphrey (mail@somnisoft.com)
 * @version 1.00
 *
 * Implementation of POSIX mailx utility in send mode.
 *
 * mailx [-s subject] [[-S option]...] [[-a attachment]...] address...
 *
 * This software has been placed into the public domain using CC0.
 */

/**
 * Required on some POSIX systems to include some standard functions.
 */
#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "smtp.h"

/**
 * Stores the to and from email addresses.
 */
struct mailx_address{
  /**
   * See @ref smtp_address_type.
   */
  enum smtp_address_type address_type;

  /**
   * Email address.
   */
  char email[1000];
};

/**
 * The attachment name and path stored for each attachment to send to the
 * recipient.
 */
struct mailx_attachment{
  /**
   * File name for this attachment to display to the recipient.
   */
  char name[1000];

  /**
   * Local file path pointing to the attachment to send.
   */
  char path[1000];
};

/**
 * The mailx context structure containing the parameters for setting up the
 * SMTP connection and sending the email.
 */
struct mailx{
  /**
   * SMTP client context.
   */
  struct smtp *smtp;

  /**
   * Email subject line.
   */
  const char *subject;

  /**
   * Email body text.
   */
  char *body;

  /**
   * SMTP server name or IP address.
   */
  char *server;

  /**
   * SMTP server port number.
   */
  char *port;

  /**
   * SMTP account user name used for authenticating.
   */
  char *user;

  /**
   * SMTP account password used for authenticating.
   */
  char *pass;

  /**
   * From email address or name.
   */
  char *from;

  /**
   * Determine if using a TLS encrypted connection or plain socket.
   */
  enum smtp_connection_security connection_security;

  /**
   * SMTP user account authentication method.
   */
  enum smtp_authentication_method auth_method;

  /**
   * Miscellaneous control flags for smtp-lib.
   *
   * See @ref smtp_flag for more details.
   */
  enum smtp_flag smtp_flags;

  /**
   * List of email addresses to send to.
   */
  struct mailx_address *address_list;

  /**
   * Number of email addresses in @ref address_list.
   */
  size_t num_address;

  /**
   * List of files to attach in the email.
   */
  struct mailx_attachment *attachment_list;

  /**
   * Number of attachments in @ref attachment_list.
   */
  size_t num_attachment;
};

/**
 * Read the entire contents of a file stream and store the data into a
 * dynamically allocated buffer.
 *
 * @param[in]  stream     File stream already opened by the caller.
 * @param[out] bytes_read Number of bytes stored in the return buffer.
 * @retval char* A dynamically allocated buffer which contains the entire
 *               contents of @p stream. The caller must free this memory
 *               when done.
 * @retval NULL Memory allocation or file read error.
 */
static char *
smtp_ffile_get_contents(FILE *stream,
                        size_t *bytes_read){
  char *read_buf;
  size_t bufsz;
  char *new_buf;
  const size_t BUFSZ_INCREMENT = 512;

  read_buf = NULL;
  bufsz = 0;

  if(bytes_read){
    *bytes_read = 0;
  }

  do{
    size_t bytes_read_loop;
    if((new_buf = realloc(read_buf, bufsz + BUFSZ_INCREMENT)) == NULL){
      free(read_buf);
      return NULL;
    }
    read_buf = new_buf;
    bufsz += BUFSZ_INCREMENT;

    bytes_read_loop = fread(&read_buf[bufsz - BUFSZ_INCREMENT],
                            sizeof(char),
                            BUFSZ_INCREMENT,
                            stream);
    if(bytes_read){
      *bytes_read += bytes_read_loop;
    }
    if(ferror(stream)){
      free(read_buf);
      return NULL;
    }
  } while(!feof(stream));

  return read_buf;
}

/**
 * Append this email to the list of email addresses to send to.
 *
 * @param[in] mailx        Append the email address into this mailx context.
 * @param[in] address_type See @ref smtp_address_type.
 * @param[in] email        Email address to send to.
 */
static void
mailx_address_append(struct mailx *const mailx,
                     enum smtp_address_type address_type,
                     const char *const email){
  struct mailx_address *new_address;
  size_t new_address_list_sz;

  mailx->num_address += 1;
  new_address_list_sz = mailx->num_address * sizeof(*mailx->address_list);
  if((mailx->address_list = realloc(mailx->address_list,
                                 new_address_list_sz)) == NULL){
    err(1, "realloc");
  }

  new_address = &mailx->address_list[mailx->num_address - 1];
  new_address->address_type = address_type;
  strncpy(new_address->email, email, sizeof(new_address->email));
  new_address->email[sizeof(new_address->email) - 1] = '\0';
}

/**
 * Send the email using the configuration options in the @p mailx context.
 *
 * @param[in] mailx Email context.
 */
static void
mailx_send(struct mailx *const mailx){
  int rc;
  size_t i;
  const struct mailx_address *address;
  const struct mailx_attachment *attachment;

  smtp_open(mailx->server,
            mailx->port,
            mailx->connection_security,
            mailx->smtp_flags,
            NULL,
            &mailx->smtp);

  smtp_auth(mailx->smtp,
            mailx->auth_method,
            mailx->user,
            mailx->pass);

  for(i = 0; i < mailx->num_address; i++){
    address = &mailx->address_list[i];
    smtp_address_add(mailx->smtp, address->address_type, address->email, NULL);
  }

  for(i = 0; i < mailx->num_attachment; i++){
    attachment = &mailx->attachment_list[i];
    smtp_attachment_add_path(mailx->smtp, attachment->name, attachment->path);
  }
  smtp_header_add(mailx->smtp, "Subject", mailx->subject);
  smtp_mail(mailx->smtp, mailx->body);

  rc = smtp_close(mailx->smtp);

  if(rc != SMTP_STATUS_OK){
    errx(1, "%s", smtp_status_code_errstr(rc));
  }
}

/**
 * Attach a file to the @p mailx context.
 *
 * @param[in] mailx    Store the attachment details into this mailx context.
 * @param[in] filename File name to display to the recipient.
 * @param[in] path     Local path of file to attach.
 */
static void
mailx_append_attachment(struct mailx *const mailx,
                        const char *const filename,
                        const char *const path){
  struct mailx_attachment *new_attachment;
  size_t new_attachment_list_sz;

  if(filename == NULL || path == NULL){
    errx(1, "must provide attachment with valid name:path");
  }

  new_attachment_list_sz = (mailx->num_attachment + 1) *
                           sizeof(*mailx->attachment_list);
  if((mailx->attachment_list = realloc(mailx->attachment_list,
                                       new_attachment_list_sz)) == NULL){
    err(1, "realloc: attachment list");
  }
  new_attachment = &mailx->attachment_list[mailx->num_attachment];
  mailx->num_attachment += 1;

  strncpy(new_attachment->name, filename, sizeof(new_attachment->name));
  new_attachment->name[sizeof(new_attachment->name) - 1] = '\0';

  strncpy(new_attachment->path, path, sizeof(new_attachment->path));
  new_attachment->path[sizeof(new_attachment->path) - 1] = '\0';
}

/**
 * Parse the file name and path and attach it to the @p mailx context.
 *
 * @param[in] mailx      Store the attachment details into this mailx context.
 * @param[in] attach_arg String with format: 'filename:filepath'.
 */
static void
mailx_append_attachment_arg(struct mailx *const mailx,
                            const char *const attach_arg){
  char *attach_arg_dup;
  char *filename;
  char *filepath;

  if((attach_arg_dup = strdup(attach_arg)) == NULL){
    err(1, "strdup: %s", attach_arg);
  }

  filename = strtok(attach_arg_dup, ":");
  filepath = strtok(NULL, ":");

  mailx_append_attachment(mailx, filename, filepath);

  free(attach_arg_dup);
}

/**
 * Parses the -S option which contains a key/value pair separated by an '='
 * character.
 *
 * @param[in] mailx  Store the results of the option parsing into the relevant
 *                   field in this mailx context.
 * @param[in] option String containing key/value option to parse.
 */
static void
mailx_parse_smtp_option(struct mailx *const mailx,
                        const char *const option){
  char *optdup;
  char *opt_key;
  char *opt_value;
  int rc;

  rc = 0;

  if((optdup = strdup(option)) == NULL){
    err(1, "strdup: option: %s", option);
  }

  if((opt_key = strtok(optdup, "=")) == NULL){
    errx(1, "strtok: %s", optdup);
  }

  opt_value = strtok(NULL, "=");

  if(strcmp(opt_key, "smtp-security") == 0){
    if(strcmp(opt_value, "none") == 0){
      mailx->connection_security = SMTP_SECURITY_NONE;
    }
#ifdef SMTP_OPENSSL
    else if(strcmp(opt_value, "tls") == 0){
      mailx->connection_security = SMTP_SECURITY_TLS;
    }
    else if(strcmp(opt_value, "starttls") == 0){
      mailx->connection_security = SMTP_SECURITY_STARTTLS;
    }
#endif /* SMTP_OPENSSL */
    else{
      rc = -1;
    }
  }
  else if(strcmp(opt_key, "smtp-auth") == 0){
    if(strcmp(opt_value, "none") == 0){
      mailx->auth_method = SMTP_AUTH_NONE;
    }
    else if(strcmp(opt_value, "plain") == 0){
      mailx->auth_method = SMTP_AUTH_PLAIN;
    }
    else if(strcmp(opt_value, "login") == 0){
      mailx->auth_method = SMTP_AUTH_LOGIN;
    }
#ifdef SMTP_OPENSSL
    else if(strcmp(opt_value, "cram-md5") == 0){
      mailx->auth_method = SMTP_AUTH_CRAM_MD5;
    }
#endif /* SMTP_OPENSSL */
    else{
      rc = -1;
    }
  }
  else if(strcmp(opt_key, "smtp-flag") == 0){
    if(strcmp(opt_value, "debug") == 0){
      mailx->smtp_flags |= SMTP_DEBUG;
    }
    else if(strcmp(opt_value, "no-cert-verify") == 0){
      mailx->smtp_flags |= SMTP_NO_CERT_VERIFY;
    }
    else{
      rc = -1;
    }
  }
  else if(strcmp(opt_key, "smtp-server") == 0){
    if((mailx->server = strdup(opt_value)) == NULL){
      err(1, "strdup");
    }
  }
  else if(strcmp(opt_key, "smtp-port") == 0){
    if((mailx->port = strdup(opt_value)) == NULL){
      err(1, "strdup");
    }
  }
  else if(strcmp(opt_key, "smtp-user") == 0){
    if((mailx->user = strdup(opt_value)) == NULL){
      err(1, "strdup");
    }
  }
  else if(strcmp(opt_key, "smtp-pass") == 0){
    if((mailx->pass = strdup(opt_value)) == NULL){
      err(1, "strdup");
    }
  }
  else if(strcmp(opt_key, "smtp-from") == 0){
    if((mailx->from = strdup(opt_value)) == NULL){
      err(1, "strdup");
    }
  }
  else{
    rc = -1;
  }

  free(optdup);

  if(rc < 0){
    errx(1, "invalid argument: %s", option);
  }
}

/**
 * Initialize and set the default options in the mailx context.
 *
 * See description of -S argument in main for more details.
 *
 * @param[in] mailx The mailx content to initialize.
 */
static void
mailx_init_default_values(struct mailx *const mailx){
  memset(mailx, 0, sizeof(*mailx));
  mailx->subject = "";
  mailx->connection_security = SMTP_SECURITY_NONE;
  mailx->auth_method = SMTP_AUTH_NONE;
}

/**
 * Frees the allocated memory associated with the mailx context.
 *
 * @param[in] mailx The mailx context to free.
 */
static void
mailx_free(const struct mailx *const mailx){
  free(mailx->body);
  free(mailx->server);
  free(mailx->port);
  free(mailx->user);
  free(mailx->pass);
  free(mailx->from);
}

/**
 * Main program entry point for the mailx utility.
 *
 * This program supports the following options:
 *   - -a 'name:path' - Attach a file with name to display to recipient and
 *                      file path pointing to file location on local storage.
 *   - -s subject     - Email subject line.
 *   - -S key=value   - A key/value pair to set various configuration options,
 *                      controlling the behavior of the SMTP client connection.
 *
 * The following list contains possible options for the -S argument:
 *   - smtp-security - none, tls, starttls
 *   - smtp-auth     - none, plain, login, cram-md5
 *   - smtp-flag     - debug, no-cert-verify
 *   - smtp-server   - server name or IP address
 *   - smtp-port     - server port number
 *   - smtp-user     - server authentication user name
 *   - smtp-pass     - server authentication user password
 *   - smtp-from     - from email account
 *
 * The following list shows the default option for -S argument if not provided:
 *   - smtp-security - none
 *   - smtp-auth     - none
 *   - smtp-flag     - none
 *   - smtp-server   - localhost
 *   - smtp-port     - 25
 *   - smtp-user     - none
 *   - smtp-pass     - none
 *   - smtp-from     - none
 *
 * @param[in] argc Number of arguments in @p argv.
 * @param[in] argv String array containing the program name and any optional
 *                 parameters described above.
 * @retval 0 Email has been sent.
 * @retval 1 An error occurred while sending email. Although unlikely, an email
 *           can still get sent even after returning with this error code.
 */
int main(int argc, char *argv[]){
  int rc;
  int i;
  struct mailx mailx;

  mailx_init_default_values(&mailx);

  while((rc = getopt(argc, argv, "a:s:S:")) != -1){
    switch(rc){
    case 'a':
      mailx_append_attachment_arg(&mailx, optarg);
      break;
    case 's':
      mailx.subject = optarg;
      break;
    case 'S':
      mailx_parse_smtp_option(&mailx, optarg);
      break;
    default:
      return 1;
    }
  }
  argc -= optind;
  argv += optind;

  if(argc < 1){
    errx(1, "must provide at least one email destination address");
  }

  if(mailx.from == NULL){
    errx(1, "must provide a FROM address");
  }

  if(mailx.server == NULL){
    if((mailx.server = strdup("localhost")) == NULL){
      err(1, "strdup");
    }
  }

  if(mailx.port == NULL){
    if((mailx.port = strdup("25")) == NULL){
      err(1, "strdup");
    }
  }

  puts("Reading email body from stdin");
  if((mailx.body = smtp_ffile_get_contents(stdin, NULL)) == NULL){
    err(1, "failed to read email body from stdin");
  }

  mailx_address_append(&mailx, SMTP_ADDRESS_FROM, mailx.from);

  for(i = 0; i < argc; i++){
    mailx_address_append(&mailx, SMTP_ADDRESS_TO, argv[i]);
  }

  mailx_send(&mailx);
  mailx_free(&mailx);
  return 0;
}

