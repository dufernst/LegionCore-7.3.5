/**
 * @file
 * @brief Test the smtp-client library without OpenSSL.
 * @author James Humphrey (mail@somnisoft.com)
 * @version 1.00
 *
 * These functional tests ensure that the smtp-client library works when
 * configured without OpenSSL.
 *
 * This software has been placed into the public domain using CC0.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <smtp.h>

/**
 * Load the test email to send from a configuration file.
 *
 * @param[out] email   String buffer to store the email in.
 * @param[in]  emailsz Number of bytes in email.
 */
static void
load_test_email(char *const email,
                size_t emailsz){
  FILE *fp;
  int rc;
  size_t bytes_read;

  fp = fopen("test/config/test_email.txt", "r");
  assert(fp);

  bytes_read = fread(email, sizeof(*email), emailsz, fp);
  assert(bytes_read > 0);

  email[bytes_read - 1] = '\0';

  rc = ferror(fp);
  assert(rc == 0);

  rc = fclose(fp);
  assert(rc == 0);
}

/**
 * Load the configuration file and send a single test email.
 */
static void
test_nossl_smtp(void){
  int rc;
  struct smtp *smtp;
  char email[1000];

  load_test_email(email, sizeof(email));

  rc = smtp_open("localhost",
                 "25",
                 SMTP_SECURITY_NONE,
                 SMTP_DEBUG,
                 NULL,
                 &smtp);
  assert(rc == SMTP_STATUS_OK);

  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_FROM,
                        email,
                        "Test Email");
  assert(rc == SMTP_STATUS_OK);

  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_TO,
                        email,
                        "Test Email");
  assert(rc == SMTP_STATUS_OK);

  rc = smtp_header_add(smtp,
                       "Subject",
                       "SMTP Test: Build Without OpenSSL");
  assert(rc == SMTP_STATUS_OK);

  rc = smtp_mail(smtp,
                 "This email tests the build without OpenSSL compiled into"
                 " the library.");
  assert(rc == SMTP_STATUS_OK);

  rc = smtp_close(smtp);
  assert(rc == SMTP_STATUS_OK);
}

/**
 * Main program entry point for testing the smtp-client library
 * build without OpenSSL.
 *
 * @retval 0 All tests passed.
 * @retval 1 Error.
 */
int main(void){
  test_nossl_smtp();
  return 0;
}

