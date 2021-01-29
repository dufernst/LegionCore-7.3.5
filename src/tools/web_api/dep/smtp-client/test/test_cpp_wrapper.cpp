/**
 * @file
 * @brief Test the smtp-client CPP wrapper.
 * @author James Humphrey (mail@somnisoft.com)
 * @version 1.00
 *
 * Example program demonstrating how to use the CPP wrapper class.
 *
 * This software has been placed into the public domain using CC0.
 */
#include <err.h>

#include <SMTPMail.h>

/**
 * Main program entry point for the example smtp-client CPP class wrapper.
 *
 * @param[in] argc Number of arguments in @p argv.
 * @param[in] argv String array containing the program name and any optional
 *                 parameters described above.
 * @retval 0 Email has been sent.
 * @retval 1 An error occurred while sending email. Although unlikely, an email
 *           can still get sent even after returning with this error code.
 */
int main(int argc, char *argv[]){
  SMTPMail *mail;

  mail = new SMTPMail();
  try{
    mail->open("localhost", "25", SMTP_SECURITY_NONE, SMTP_DEBUG, NULL);
    mail->auth(SMTP_AUTH_NONE, NULL, NULL);
    mail->address_add(SMTP_ADDRESS_FROM,
                      "mail@somnisoft.com",
                      "From Address");
    mail->address_add(SMTP_ADDRESS_TO,
                      "mail@somnisoft.com",
                      "To Address");
    mail->header_add("Subject", "Test email (SMTPMail)");
    mail->mail("Email sent using CPP SMTPMail class");
    mail->close();
  }
  catch(SMTPMailException sme){
    errx(1, "Failed to send email: %s\n", sme.what());
  }
  delete mail;

  return 0;
}

