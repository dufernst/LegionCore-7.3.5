#include <stdio.h>
#include "smtp.h"
#define MAIL_SERVER              "localhost"
#define MAIL_PORT                "587"
#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_NONE
#define MAIL_FLAGS               SMTP_DEBUG
#define MAIL_CAFILE              NULL
#define MAIL_AUTH                SMTP_AUTH_NONE
#define MAIL_USER                "mail@somnisoft.com"
#define MAIL_PASS                "password"
#define MAIL_FROM                "mail@somnisoft.com"
#define MAIL_FROM_NAME           "From Name"
#define MAIL_SUBJECT             "Subject Line"
#define MAIL_TO                  "mail@somnisoft.com"
#define MAIL_TO_NAME             "To Name"
int main(void){
  struct smtp *smtp;
  enum smtp_status_code rc;
  const char *const email_body =
  "<html>\n"
  " <head><title>HTML Email</title></head>\n"
  " <body>\n"
  "  <h1>H1</h1>\n"
  "  <h2>H2</h1>\n"
  "  <h3>H3</h1>\n"
  "  <h4>H4</h1>\n"
  "  <h5>H5</h1>\n"
  "  <h6>H6</h1>\n"
  " </body>\n"
  "</html>\n";
  smtp_open(MAIL_SERVER,
            MAIL_PORT,
            MAIL_CONNECTION_SECURITY,
            MAIL_FLAGS,
            MAIL_CAFILE,
            &smtp);
  smtp_auth(smtp,
            MAIL_AUTH,
            MAIL_USER,
            MAIL_PASS);
  smtp_address_add(smtp,
                   SMTP_ADDRESS_FROM,
                   MAIL_FROM,
                   MAIL_FROM_NAME);
  smtp_address_add(smtp,
                   SMTP_ADDRESS_TO,
                   MAIL_TO,
                   MAIL_TO_NAME);
  smtp_header_add(smtp,
                  "Subject",
                  MAIL_SUBJECT);
  smtp_header_add(smtp,
                  "Content-Type",
                  "text/html");
  smtp_mail(smtp, email_body);
  rc = smtp_close(smtp);
  if(rc != SMTP_STATUS_OK){
    fprintf(stderr, "smtp failed: %s\n", smtp_status_code_errstr(rc));
    return 1;
  }
  return 0;
}
