/**
 * @file
 * @brief Test the smtp-client library.
 * @author James Humphrey (mail@somnisoft.com)
 * @version 1.00
 *
 * This smtp-client testing framework has 100% branch coverage on POSIX
 * systems. It requires a Postfix SMTP server that supports all of the
 * connection security and authentication methods. These functional tests
 * also require the user to manually check and ensure that the destination
 * addresses received all of the test emails.
 *
 * This software has been placed into the public domain using CC0.
 *
 * @section test_seams_countdown_global
 *
 * The test harnesses control most of the test seams through the use of
 * global counter values.
 *
 * Setting a global counter to -1 will make the test seam function operate
 * as it normally would. If set to a positive value, the value will continue
 * to decrement every time the function gets called. When the counter reaches
 * 0, the test seam will force the function to return an error value.
 *
 * For example, initially setting the counter to 0 will force the test seam
 * to return an error condition the first time it gets called. Setting the
 * value to 1 initially will force the test seam to return an error condition
 * on the second time it gets called.
 */
#ifndef SMTP_TEST_H
#define SMTP_TEST_H

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../src/smtp.h"

#ifdef SMTP_OPENSSL
# include <openssl/bio.h>
# include <openssl/err.h>
# include <openssl/ssl.h>
# include <openssl/x509.h>
# include <openssl/x509v3.h>
#endif /* SMTP_OPENSSL */

struct smtp_command;
struct str_getdelimfd;

int
smtp_si_add_size_t(const size_t a,
                   const size_t b,
                   size_t *const result);

int
smtp_si_sub_size_t(const size_t a,
                   const size_t b,
                   size_t *const result);

int
smtp_si_mul_size_t(const size_t a,
                   const size_t b,
                   size_t *const result);

size_t
smtp_base64_decode(const char *const buf,
                   unsigned char **decode);

char *
smtp_base64_encode(const char *const buf,
                   size_t buflen);

char *
smtp_bin2hex(const unsigned char *const s,
             size_t slen);

enum smtp_status_code
smtp_write(struct smtp *const smtp,
           const char *const buf,
           size_t len);

int
smtp_str_getdelimfd(struct str_getdelimfd *const gdfd);

int
smtp_str_getdelimfd_set_line_and_buf(struct str_getdelimfd *const gdfd,
                                     size_t copy_len);

void
smtp_str_getdelimfd_free(struct str_getdelimfd *const gdfd);

char *
smtp_stpcpy(char *s1,
            const char *s2);

void *
smtp_reallocarray(void *ptr,
                  size_t nmemb,
                  size_t size);

char *
smtp_strdup(const char *s);

char *
smtp_str_replace(const char *const search,
            const char *const replace,
            const char *const s);

size_t
smtp_utf8_charlen(char c);

int
smtp_str_has_nonascii_utf8(const char *const s);

size_t
smtp_strnlen_utf8(const char *s,
                  size_t maxlen);

size_t
smtp_fold_whitespace_get_offset(const char *const s,
                                unsigned int maxlen);

char *
smtp_fold_whitespace(const char *const s,
                     unsigned int maxlen);

char *
smtp_chunk_split(const char *const s,
                 size_t chunklen,
                 const char *const end);

char *
smtp_ffile_get_contents(FILE *stream,
                        size_t *bytes_read);

char *
smtp_file_get_contents(const char *const filename,
                       size_t *bytes_read);

int
smtp_parse_cmd_line(char *const line,
                    struct smtp_command *const cmd);

int
smtp_date_rfc_2822(char *const date);

int
smtp_address_validate_email(const char *const email);

int
smtp_address_validate_name(const char *const name);

int
smtp_attachment_validate_name(const char *const name);

int
smtp_header_key_validate(const char *const key);

int
smtp_header_value_validate(const char *const value);

/* test seams */
int
smtp_test_seam_dec_err_ctr(int *const test_err_ctr);

BIO *
smtp_test_seam_bio_new_socket(int sock,
                              int close_flag);

int
smtp_test_seam_bio_should_retry(BIO *bio);

void *
smtp_test_seam_calloc(size_t nelem,
                      size_t elsize);

int
smtp_test_seam_close(int fildes);

int
smtp_test_seam_connect(int socket,
                       const struct sockaddr *address,
                       socklen_t address_len);

unsigned long
smtp_test_seam_err_peek_error(void);

int
smtp_test_seam_fclose(FILE *stream);

int
smtp_test_seam_ferror(FILE *stream);

struct tm *
smtp_test_seam_gmtime_r(const time_t *timep,
                        struct tm *result);

unsigned char *
smtp_test_seam_hmac(const EVP_MD *evp_md,
                    const void *key,
                    int key_len,
                    const unsigned char *d,
                    size_t n,
                    unsigned char *md,
                    unsigned int *md_len);

struct tm *
smtp_test_seam_localtime_r(const time_t *timep,
                           struct tm *result);

void *
smtp_test_seam_malloc(size_t size);

time_t
smtp_test_seam_mktime(struct tm *timeptr);


void *
smtp_test_seam_realloc(void *ptr,
                       size_t size);

long
smtp_test_seam_recv(int socket,
                    void *buffer,
                    size_t length,
                    int flags);

int
smtp_test_seam_select(int nfds,
                      fd_set *readfds,
                      fd_set *writefds,
                      fd_set *errorfds,
                      struct timeval *timeout);

ssize_t
smtp_test_seam_send(int socket,
                    const void *buffer,
                    size_t length,
                    int flags);

int
smtp_test_seam_socket(int domain,
                      int type,
                      int protocol);

int
smtp_test_seam_ssl_connect(SSL *ssl);

SSL_CTX *
smtp_test_seam_ssl_ctx_new(const SSL_METHOD *method);

int
smtp_test_seam_ssl_do_handshake(SSL *ssl);

X509 *
smtp_test_seam_ssl_get_peer_certificate(const SSL *ssl);

int
smtp_test_seam_x509_check_host(X509 *cert,
                               const char *name,
                               size_t namelen,
                               unsigned int flags,
                               char **peername);

SSL *
smtp_test_seam_ssl_new(SSL_CTX *ctx);

int
smtp_test_seam_ssl_read(SSL *ssl,
                        void *buf,
                        int num);

int
smtp_test_seam_ssl_write(SSL *ssl,
                         const void *buf,
                         int num);

int
smtp_test_seam_sprintf(char *s,
                       const char *format, ...);

size_t
smtp_test_seam_strlen(const char *s);

time_t
smtp_test_seam_time(time_t *tloc);

/**
 * Counter for @ref smtp_test_seam_bio_new_socket.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_bio_new_socket_ctr;

/**
 * Counter for @ref smtp_test_seam_bio_should_retry.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_bio_should_retry_ctr;

/**
 * Value to force the BIO_should_retry() function to return.
 *
 * This value will only get returned if
 * @ref g_smtp_test_err_bio_should_retry_ctr
 * has a value of 0 and this does not have a value of -1.
 */
extern int g_smtp_test_err_bio_should_retry_rc;

/**
 * Counter for @ref smtp_test_seam_calloc.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_calloc_ctr;

/**
 * Counter for @ref smtp_test_seam_close.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_close_ctr;

/**
 * Counter for @ref smtp_test_seam_connect.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_connect_ctr;

/**
 * Counter for @ref smtp_test_seam_err_peek_error.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_err_peek_error_ctr;

/**
 * Counter for @ref smtp_test_seam_fclose.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_fclose_ctr;

/**
 * Counter for @ref smtp_test_seam_ferror.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ferror_ctr;

/**
 * Counter for @ref smtp_test_seam_gmtime_r.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_gmtime_r_ctr;

/**
 * Counter for @ref smtp_test_seam_hmac.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_hmac_ctr;

/**
 * Counter for @ref smtp_test_seam_localtime_r.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_localtime_r_ctr;

/**
 * Counter for @ref smtp_test_seam_malloc.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_malloc_ctr;

/**
 * Counter for @ref smtp_test_seam_mktime.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_mktime_ctr;

/**
 * Counter for @ref smtp_test_seam_realloc.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_realloc_ctr;

/**
 * Counter for @ref smtp_test_seam_recv.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_recv_ctr;

/**
 * Value to force the recv() function to return.
 *
 * This value will only get returned if
 * @ref g_smtp_test_err_recv_ctr
 * has a value of 0 and this does not have a value of -1.
 */
extern int g_smtp_test_err_recv_rc;

/**
 * Set the received bytes in recv() and SSL_read() to this value if it
 * contains a null-terminated string at least one bytes long.
 *
 * This makes it easier to inject a bad server response for testing the
 * smtp-client handling of those bad responses.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern char g_smtp_test_err_recv_bytes[90];

/**
 * Counter for @ref smtp_test_seam_select.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_select_ctr;

/**
 * Counter for @ref smtp_test_seam_send.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_send_ctr;

/**
 * Indicate if we should only send one byte at a time.
 */
extern int g_smtp_test_send_one_byte;

/**
 * Counter for @ref smtp_si_add_size_t.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_si_add_size_t_ctr;

/**
 * Counter for @ref smtp_si_sub_size_t.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_si_sub_size_t_ctr;

/**
 * Counter for @ref smtp_si_mul_size_t.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_si_mul_size_t_ctr;

/**
 * Counter for @ref smtp_test_seam_socket.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_socket_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_connect.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_connect_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_ctx_new.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_ctx_new_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_do_handshake.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_do_handshake_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_get_peer_certificate.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_get_peer_certificate_ctr;

/**
 * Counter for @ref smtp_test_seam_x509_check_host.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_x509_check_host_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_new.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_new_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_read.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_read_ctr;

/**
 * Counter for @ref smtp_test_seam_ssl_write.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_ssl_write_ctr;

/**
 * Counter for @ref smtp_test_seam_sprintf.
 *
 * See @ref test_seams_countdown_global for more details.
 */
extern int g_smtp_test_err_sprintf_ctr;

/**
 * Value to force the sprintf() function to return.
 *
 * This value will only get returned if @ref g_smtp_test_err_sprintf_ctr has
 * a value of 0.
 */
extern int g_smtp_test_err_sprintf_rc;

/**
 * Indicates if the strlen() function should return a test value.
 *
 * This can get set to one of two values:
 *   -  0 - The strlen() function will operate normally.
 *   - !0 - The strlen() function will return the value specified in
 *          @ref g_smtp_test_strlen_ret_value.
 */
extern int g_smtp_test_strlen_custom_ret;

/**
 * Value to force the strlen() function to return.
 *
 * This value will only get returned if @ref g_smtp_test_strlen_custom_ret
 * has been set.
 */
extern size_t g_smtp_test_strlen_ret_value;

/**
 * Indicates if the time() function should return a custom value.
 *
 * This can get set to one of two values:
 *   -  0 - The time() function will operate normally.
 *   - !0 - The time() function will return the value specified in
 *          @ref g_smtp_test_time_ret_value.
 */
extern int g_smtp_test_time_custom_ret;

/**
 * Value to force the time() function to return.
 *
 * This value will only get returned if @ref g_smtp_test_time_custom_ret has
 * a positive value.
 */
extern time_t g_smtp_test_time_ret_value;

#endif /* SMTP_TEST_H */

