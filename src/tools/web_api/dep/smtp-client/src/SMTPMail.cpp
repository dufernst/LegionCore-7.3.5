/**
 * @file
 * @brief SMTPMail class wrapper for smtp-client library.
 * @author James Humphrey (mail@somnisoft.com)
 * @version 1.00
 *
 * Thin CPP wrapper class around the smtp-client C library.
 *
 * This software has been placed into the public domain using CC0.
 */
#include "SMTPMail.h"

SMTPMailException::SMTPMailException(enum smtp_status_code status_code){
  this->status_code = status_code;
}

const char*
SMTPMailException::what() const noexcept {
  return smtp_status_code_errstr(this->status_code);
}

SMTPMail::SMTPMail(void){
}

SMTPMail::~SMTPMail(void){
}

void SMTPMail::open(const char *const server,
                    const char *const port,
                    enum smtp_connection_security connection_security,
                    enum smtp_flag flags,
                    const char *const cafile){
  this->rc = smtp_open(server,
                       port,
                       connection_security,
                       flags,
                       cafile,
                       &this->smtp);
  this->throw_bad_status_code();
}

void SMTPMail::auth(enum smtp_authentication_method auth_method,
                    const char *const user,
                    const char *const pass){
  this->rc = smtp_auth(this->smtp, auth_method, user, pass);
  this->throw_bad_status_code();
}

void SMTPMail::mail(const char *const body){
  this->rc = smtp_mail(this->smtp, body);
  this->throw_bad_status_code();
}

void SMTPMail::close(void){
  this->rc = smtp_close(this->smtp);
  this->throw_bad_status_code();
}

int SMTPMail::status_code_get(void){
  return smtp_status_code_get(this->smtp);
}

void SMTPMail::status_code_set(enum smtp_status_code new_status_code){
  this->rc = smtp_status_code_set(this->smtp, new_status_code);
  this->throw_bad_status_code();
}

void SMTPMail::header_add(const char *const key,
                          const char *const value){
  this->rc = smtp_header_add(this->smtp, key, value);
  this->throw_bad_status_code();
}

void SMTPMail::header_clear_all(void){
  smtp_header_clear_all(this->smtp);
}

void SMTPMail::address_add(enum smtp_address_type type,
                           const char *const email,
                           const char *const name){
  this->rc = smtp_address_add(this->smtp, type, email, name);
  this->throw_bad_status_code();
}

void SMTPMail::address_clear_all(void){
  smtp_address_clear_all(this->smtp);
}

void SMTPMail::attachment_add_path(const char *const name,
                                   const char *const path){
  this->rc = smtp_attachment_add_path(this->smtp, name, path);
  this->throw_bad_status_code();
}

void SMTPMail::attachment_add_fp(const char *const name,
                                 FILE *fp){
  this->rc = smtp_attachment_add_fp(this->smtp, name, fp);
  this->throw_bad_status_code();
}

void SMTPMail::attachment_add_mem(const char *const name,
                                  const void *const data,
                                  size_t datasz){
  this->rc = smtp_attachment_add_mem(this->smtp, name, data, datasz);
  this->throw_bad_status_code();
}

void SMTPMail::attachment_clear_all(void){
  smtp_attachment_clear_all(this->smtp);
}

void SMTPMail::throw_bad_status_code(void){
  if(this->rc != SMTP_STATUS_OK){
    throw SMTPMailException(this->rc);
  }
}

