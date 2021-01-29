# smtp-client

This is an SMTP client library written in C which can get included
directly into another program.

This library has been released into the public domain using
[CC0](https://creativecommons.org/publicdomain/zero/1.0/).

Official repository location:
[www.somnisoft.com/smtp-client](https://www.somnisoft.com/smtp-client)

## Feature list
* C89
* Cross-platform (POSIX, BSD, MacOS, Windows)
* Send attachments
* Send custom email headers
* Specify multiple TO, CC, and BCC recipients
* Simple API and simple error handling (see Examples section below)
* Optional OpenSSL TLS connection and authentication methods
* Test cases with 100% code and branch coverage
* Doxygen with 100% documentation including the test code
* Free software (permissive - CC0)

Supports the following connection methods:
* No encryption
* STARTTLS (requires OpenSSL)
* TLS direct connection (requires OpenSSL)

Supports the following authentication methods:
* none
* PLAIN
* LOGIN
* CRAM-MD5 (requires OpenSSL)

To include the library into your application, simply copy the src/smtp.h and
src/smtp.c files into your project directory. Then include the smtp.h header
into your C file, compile smtp.c, and include the resulting object file into
the build system.

## Examples
The following example code demonstrates how to use the library.

```C
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
```

Place the code snippet above into a file named 'test.c' and change each #define
to the appropriate values for your mail server. Then copy smtp.c and smtp.h
into the same directory and run the following commands to compile with OpenSSL
support.

cc -DSMTP_OPENSSL smtp.c -c -o smtp.o

cc -DSMTP_OPENSSL test.c -c -o test.o

cc test.o smtp.o -o smtp_test -lssl -lcrypto

If you do not need OpenSSL support, remove the -DSMTP_OPENSSL and the
-lssl and -lcrypto arguments. The commands as above should create an
executable called 'smtp_test' which can send a test email using the specified
mail server.

A minimum amount of error checking has been included. Note that in the above
example, the program checks for an error at the end of the mail session after
calling smtp_close. We can do this because if an error occurs in any of the
prior functions, the error will continue to propagate through any future
function calls until either closing the SMTP context using smtp_close or by
resetting the error condition using smtp_status_code_clear.

The following example demonstrates how to send an HTML email by overriding the
Content-Type header. When overriding this header, any attachments added using
the smtp_attachment_add\* functions will get ignored. The application must
generate the appropriate MIME sections (if needed) when overriding this
header.

```C
#include <stdio.h>
#include "smtp.h"
#define MAIL_SERVER              "localhost"
#define MAIL_PORT                "25"
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
```

## Technical Documentation
See the [Technical Documentation](
https://www.somnisoft.com/smtp-client/technical-documentation/index.html)
generated from Doxygen.

