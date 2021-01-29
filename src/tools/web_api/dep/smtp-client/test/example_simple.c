#include <stdio.h>
#include "smtp.h"
#define MAIL_SERVER              "mail.example.com"
#define MAIL_PORT                "587"
#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_STARTTLS
#define MAIL_FLAGS               (SMTP_DEBUG         | \
                                  SMTP_NO_CERT_VERIFY) /* Do not verify cert. */
#define MAIL_CAFILE              NULL
#define MAIL_AUTH                SMTP_AUTH_PLAIN
#define MAIL_USER                "mail@example.com"
#define MAIL_PASS                "password"
#define MAIL_FROM                "mail@example.com"
#define MAIL_FROM_NAME           "From Name"
#define MAIL_SUBJECT             "Subject Line"
#define MAIL_BODY                "Email Body"
#define MAIL_TO                  "to@example.com"
#define MAIL_TO_NAME             "To Name"
int main(void)
{
  struct smtp *smtp;
  int rc;
  rc = smtp_open(MAIL_SERVER,
                 MAIL_PORT,
                 MAIL_CONNECTION_SECURITY,
                 MAIL_FLAGS,
                 MAIL_CAFILE,
                 &smtp);
  rc = smtp_auth(smtp,
                 MAIL_AUTH,
                 MAIL_USER,
                 MAIL_PASS);
  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_FROM,
                        MAIL_FROM,
                        MAIL_FROM_NAME);
  rc = smtp_address_add(smtp,
                        SMTP_ADDRESS_TO,
                        MAIL_TO,
                        MAIL_TO_NAME);
  rc = smtp_header_add(smtp,
                       "Subject",
                       MAIL_SUBJECT);
  rc = smtp_attachment_add_mem(smtp,
                               "test.txt",
                               "Test email attachment.",
                               -1);
  rc = smtp_mail(smtp,
                 MAIL_BODY);
  rc = smtp_close(smtp);
  if(rc != SMTP_STATUS_OK){
    fprintf(stderr, "smtp failed: %s\n", smtp_status_code_errstr(rc));
    return 1;
  }
  return 0;
}
