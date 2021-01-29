/**
 * @file
 * @brief Test seams for the smtp-client library.
 * @author James Humphrey (mail@somnisoft.com)
 * @version 1.00
 *
 * Used by the smtp-client testing framework to inject specific return values
 * by some standard library functions. This makes it possible to test less
 * common errors like out of memory conditions and input/output errors.
 *
 * This software has been placed into the public domain using CC0.
 */
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "test.h"

/**
 * See @ref g_smtp_test_err_bio_new_socket_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_bio_new_socket_ctr = -1;

/**
 * See @ref g_smtp_test_err_bio_should_retry_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_bio_should_retry_ctr = -1;

/**
 * See @ref g_smtp_test_err_bio_should_retry_rc.
 */
int g_smtp_test_err_bio_should_retry_rc = -1;

/**
 * See @ref g_smtp_test_err_calloc_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_calloc_ctr = -1;

/**
 * See @ref g_smtp_test_err_close_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_close_ctr = -1;

/**
 * See @ref g_smtp_test_err_connect_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_connect_ctr = -1;

/**
 * See @ref g_smtp_test_err_err_peek_error_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_err_peek_error_ctr = -1;

/**
 * See @ref g_smtp_test_err_fclose_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_fclose_ctr = -1;

/**
 * See @ref g_smtp_test_err_ferror_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ferror_ctr = -1;

/**
 * See @ref g_smtp_test_err_gmtime_r_ctr @ref test_seams_countdown_global.
 */
int g_smtp_test_err_gmtime_r_ctr = -1;

/**
 * See @ref g_smtp_test_err_hmac_ctr @ref test_seams_countdown_global.
 */
int g_smtp_test_err_hmac_ctr = -1;

/**
 * See @ref g_smtp_test_err_localtime_r_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_localtime_r_ctr = -1;

/**
 * See @ref g_smtp_test_err_malloc_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_malloc_ctr = -1;

/**
 * See @ref g_smtp_test_err_mktime_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_mktime_ctr = -1;

/**
 * See @ref g_smtp_test_err_realloc_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_realloc_ctr = -1;

/**
 * See @ref g_smtp_test_err_recv_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_recv_ctr = -1;

/**
 * See @ref g_smtp_test_err_recv_rc.
 */
int g_smtp_test_err_recv_rc = -1;

/**
 * See @ref g_smtp_test_err_recv_bytes and @ref test_seams_countdown_global.
 */
char g_smtp_test_err_recv_bytes[90] = {0};

/**
 * See @ref g_smtp_test_err_select_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_select_ctr = -1;

/**
 * See @ref g_smtp_test_err_send_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_send_ctr = -1;

/**
 * See @ref g_smtp_test_send_one_byte.
 */
int g_smtp_test_send_one_byte = 0;

/**
 * See @ref g_smtp_test_err_si_add_size_t_ctr
 * and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_si_add_size_t_ctr = -1;

/**
 * See @ref g_smtp_test_err_si_sub_size_t_ctr
 * and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_si_sub_size_t_ctr = -1;

/**
 * See @ref g_smtp_test_err_si_mul_size_t_ctr
 * and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_si_mul_size_t_ctr = -1;

/**
 * See @ref g_smtp_test_err_socket_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_socket_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_connect_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_connect_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_ctx_new_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_ctx_new_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_do_handshake_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_do_handshake_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_get_peer_certificate_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_get_peer_certificate_ctr = -1;

/**
 * See @ref g_smtp_test_err_x509_check_host_ctr and
 * @ref test_seams_countdown_global.
 */
int g_smtp_test_err_x509_check_host_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_new_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_new_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_read_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_read_ctr = -1;

/**
 * See @ref g_smtp_test_err_ssl_write_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_ssl_write_ctr = -1;

/**
 * See @ref g_smtp_test_err_sprintf_ctr and @ref test_seams_countdown_global.
 */
int g_smtp_test_err_sprintf_ctr = -1;

/**
 * See @ref g_smtp_test_err_sprintf_rc.
 */
int g_smtp_test_err_sprintf_rc = 0;

/**
 * See @ref g_smtp_test_strlen_custom_ret.
 */
int g_smtp_test_strlen_custom_ret = 0;

/**
 * See @ref g_smtp_test_strlen_ret_value.
 */
size_t g_smtp_test_strlen_ret_value = 0;

/**
 * See @ref g_smtp_test_time_custom_ret.
 */
int g_smtp_test_time_custom_ret = 0;

/**
 * See @ref g_smtp_test_time_ret_value.
 */
time_t g_smtp_test_time_ret_value = 0;

/**
 * Decrement an error counter until it reaches -1.
 *
 * Once a counter reaches -1, it will return a successful response (1). This
 * typically gets used to denote when to cause a function to fail. For example,
 * the unit test or functional test might need to cause the realloc() function
 * to fail after calling it the third time.
 *
 * @param[in,out] test_err_ctr Integer counter to decrement.
 * @retval 0 The counter has been decremented, but did not reach -1 yet.
 * @retval 1 The counter has reached -1.
 */
int
smtp_test_seam_dec_err_ctr(int *const test_err_ctr){
  if(*test_err_ctr >= 0){
    *test_err_ctr -= 1;
    if(*test_err_ctr < 0){
      return 1;
    }
  }
  return 0;
}

/**
 * Allows the test harness to control when BIO_new_socket() fails.
 *
 * @param[in] sock       Existing socket to attach the BIO to.
 * @param[in] close_flag Close flag for new BIO.
 * @retval BIO* New BIO created on existing socket.
 * @retval NULL Failed to create the new BIO.
 */
BIO *
smtp_test_seam_bio_new_socket(int sock,
                              int close_flag){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_bio_new_socket_ctr)){
    return NULL;
  }
  return BIO_new_socket(sock, close_flag);
}

/**
 * Allows the test harness to control when BIO_should_retry() fails.
 *
 * @param[in] bio Existing BIO connection.
 * @retval 0 The error condition does not allow a retry.
 * @retval 1 The error condition allows a retry.
 */
int
smtp_test_seam_bio_should_retry(BIO *bio){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_bio_should_retry_ctr)){
    return 0;
  }
  if(g_smtp_test_err_bio_should_retry_rc != -1){
    return g_smtp_test_err_bio_should_retry_rc;
  }
  return BIO_should_retry(bio);
}

/**
 * Allows the test harness to control when calloc() fails.
 *
 * @param[in] nelem  Number of elements to allocate.
 * @param[in] elsize Size of each element to allocate.
 * @retval void* Pointer to new allocated memory.
 * @retval NULL  Memory allocation failed.
 */
void *
smtp_test_seam_calloc(size_t nelem,
                      size_t elsize){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_calloc_ctr)){
    errno = ENOMEM;
    return NULL;
  }
  return calloc(nelem, elsize);
}

/**
 * Allows the test harness to control when close() fails.
 *
 * @param[in] fildes Socket file descriptor to close.
 * @retval  0 Successfully closed file descriptor.
 * @retval -1 Failed to close file descriptor.
 */
int
smtp_test_seam_close(int fildes){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_close_ctr)){
    errno = EBADF;
    return -1;
  }
  return close(fildes);
}

/**
 * Allows the test harness to control when connect() fails.
 *
 * @param[in] socket      Socket connection.
 * @param[in] address     Network address of peer.
 * @param[in] address_len Number of bytes in @p address.
 * @retval 0  Successfully connected to the peer.
 * @retval -1 Failed to connect to the peer.
 */
int
smtp_test_seam_connect(int socket,
                       const struct sockaddr *address,
                       socklen_t address_len){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_connect_ctr)){
    errno = ECONNREFUSED;
    return -1;
  }
  return connect(socket, address, address_len);
}

/**
 * Allows the test harness to control when ERR_peek_error() returns a failure
 * code.
 *
 * @retval  0 No error code on the error queue.
 * @retval !0 An error code exists on the error queue.
 */
unsigned long
smtp_test_seam_err_peek_error(void){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_err_peek_error_ctr)){
    return 1;
  }
  return ERR_peek_error();
}

/**
 * Allows the test harness to control when fclose() fails.
 *
 * @param[in] stream File stream to close.
 * @retval 0   Successfully closed the file stream.
 * @retval EOF An error occurred while closing the file stream.
 */
int smtp_test_seam_fclose(FILE *stream){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_fclose_ctr)){
    errno = EBADF;
    return EOF;
  }
  return fclose(stream);
}

/**
 * Allows the test harness to control the file stream error indicator return
 * value in ferror().
 *
 * @param[in] stream Check for errors on this file stream.
 * @retval 0 No errors detected on the file stream.
 * @retval 1 An error occurred during a file stream operation.
 */
int
smtp_test_seam_ferror(FILE *stream){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ferror_ctr)){
    return 1;
  }
  return ferror(stream);
}

/**
 * Allows the test harness to control when gmtime_r() fails.
 *
 * @param[in]  timep  Time value to convert to a struct tm.
 * @param[out] result Converts the @p timep value into a UTC tm structure
 *                    value and stores the results in this pointer.
 * @retval tm*  time_t value converted to a tm structure value.
 * @retval NULL An error occurred while converting the time.
 */
struct tm *
smtp_test_seam_gmtime_r(const time_t *timep,
                        struct tm *result){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_gmtime_r_ctr)){
    return NULL;
  }
  return gmtime_r(timep, result);
}

/**
 * Allows the test harness to control when HMAC() fails.
 *
 * @param[in]  evp_md  Hash function.
 * @param[in]  key     Hash key.
 * @param[in]  key_len Number of bytes in @p key.
 * @param[in]  d       Message data.
 * @param[in]  n       Number of bytes in @p d.
 * @param[out] md      The computed message authentication code.
 * @param[in]  md_len  Number of bytes in @p md.
 * @retval uchar* Pointer to @p md.
 * @retval NULL   An error occurred.
 */
unsigned char *
smtp_test_seam_hmac(const EVP_MD *evp_md,
                    const void *key,
                    int key_len,
                    const unsigned char *d,
                    size_t n,
                    unsigned char *md,
                    unsigned int *md_len){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_hmac_ctr)){
    return NULL;
  }
  return HMAC(evp_md, key, key_len, d, n, md, md_len);
}

/**
 * Allows the test harness to control when localtime_r() fails.
 *
 * @param[in]  timep  Time value to convert to a struct tm.
 * @param[out] result Converts the @p timep value into a local time tm
 *                    structure value and stores the results in this pointer.
 * @retval tm*  time_t value converted to a tm structure value.
 * @retval NULL An error occurred while converting the time.
 */
struct tm *
smtp_test_seam_localtime_r(const time_t *timep,
                           struct tm *result){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_localtime_r_ctr)){
    return NULL;
  }
  return localtime_r(timep, result);
}

/**
 * Allows the test harness to control when malloc() fails.
 *
 * @param[in] size Number of bytes to allocate.
 * @retval void* Pointer to new allocated memory.
 * @retval NULL  Memory allocation failed.
 */
void *
smtp_test_seam_malloc(size_t size){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_malloc_ctr)){
    errno = ENOMEM;
    return NULL;
  }
  return malloc(size);
}

/**
 * Allows the test harness to control when mktime() fails.
 *
 * @param[in] timeptr tm data structure to convert to time_t.
 * @retval >=0 Time since the epoch.
 * @retval -1  Failed to convert the time.
 */
time_t
smtp_test_seam_mktime(struct tm *timeptr){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_mktime_ctr)){
    return -1;
  }
  return mktime(timeptr);
}

/**
 * Allows the test harness to control when realloc() fails.
 *
 * @param[in] ptr  Previously allocated memory or NULL memory has not been
 *                 allocated yet.
 * @param[in] size Number of bytes to reallocate.
 * @retval void* Pointer to new allocated memory.
 * @retval NULL  Memory allocation failed.
 */
void *
smtp_test_seam_realloc(void *ptr,
                       size_t size){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_realloc_ctr)){
    errno = ENOMEM;
    return NULL;
  }
  return realloc(ptr, size);
}

/**
 * Allows the test harness to control when recv() fails.
 *
 * @param[in] socket TCP network socket.
 * @param[in] buffer Store received data in this buffer.
 * @param[in] length Number of bytes in @p buffer.
 * @param[in] flags  Set this to 0.
 * @retval >=0 Number of bytes received.
 * @retval  -1 Failed to receive bytes over the network.
 */
long
smtp_test_seam_recv(int socket,
                    void *buffer,
                    size_t length,
                    int flags){
  size_t bytes_inject_len;

  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_recv_ctr)){
    if(g_smtp_test_err_recv_rc != -1){
      return g_smtp_test_err_recv_rc;
    }
    if(*g_smtp_test_err_recv_bytes){
      bytes_inject_len = strlen(g_smtp_test_err_recv_bytes);
      assert(bytes_inject_len < length && bytes_inject_len < LONG_MAX);
      memcpy(buffer, g_smtp_test_err_recv_bytes, bytes_inject_len);
      return (long)bytes_inject_len;
    }
    errno = EBADF;
    return -1;
  }
  return recv(socket, buffer, length, flags);
}

/**
 * Allows the test harness to control when select() fails.
 *
 * @param[in] nfds     Check for file descriptors in range 0 to (@p nfds - 1)
 *                     which have any of the read/write/error conditions.
 * @param[in] readfds  Checks for file descriptors in fd_set that have bytes
 *                     ready for reading.
 * @param[in] writefds Checks for file descriptors in fd_set that have bytes
 *                     ready for writing.
 * @param[in] errorfds Checks for file descriptors in fd_set that have errors
 *                     pending.
 * @param[in] timeout  Wait for the read/write/error conditions in blocking
 *                     mode until this timeout or an interrupt occurs.
 * @retval >=0 Number of bits set in the bitmask.
 * @retval  -1 An error occurred.
 */
int
smtp_test_seam_select(int nfds,
                      fd_set *readfds,
                      fd_set *writefds,
                      fd_set *errorfds,
                      struct timeval *timeout){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_select_ctr)){
    errno = EINTR;
    return -1;
  }
  return select(nfds, readfds, writefds, errorfds, timeout);
}

/**
 * Allows the test harness to control when send() fails.
 *
 * @param[in] socket TCP network socket.
 * @param[in] buffer Data to send over the network.
 * @param[in] length Number of bytes in @p buffer.
 * @param[in] flags  Set this to 0.
 * @retval >=0 Number of bytes sent.
 * @retval  -1 Failed to send bytes over the network.
 */
ssize_t
smtp_test_seam_send(int socket,
                    const void *buffer,
                    size_t length,
                    int flags){
  long sent_bytes;
  size_t bytes_to_send;

  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_send_ctr)){
    errno = EBADF;
    sent_bytes = -1;
  }
  else{
    bytes_to_send = length;
    if(g_smtp_test_send_one_byte){
      bytes_to_send = 1;
    }
    sent_bytes = send(socket, buffer, bytes_to_send, flags);
  }
  return sent_bytes;
}

/**
 * Allows the test harness to control when socket() fails.
 *
 * @param[in] domain   Socket domain.
 * @param[in] type     Socket type.
 * @param[in] protocol Socket protocol.
 * @retval !(-1) The file descriptor for the new socket.
 * @retval   -1  Failed to create the socket.
 */
int
smtp_test_seam_socket(int domain,
                      int type,
                      int protocol){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_socket_ctr)){
    errno = EINVAL;
    return -1;
  }
  return socket(domain, type, protocol);
}

/**
 * Allows the test harness to control when SSL_connect() fails.
 *
 * @param[in] ssl OpenSSL handle.
 * @retval  1 TLS connection handshake successful.
 * @retval <1 TLS connection handshake failed.
 */
int
smtp_test_seam_ssl_connect(SSL *ssl){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_connect_ctr)){
    return 0;
  }
  return SSL_connect(ssl);
}

/**
 * Allows the test harness to control when SSL_CTX_new() fails.
 *
 * @param[in] method TLS connection method.
 * @retval SSL_CTX* Pointer to new TLS context.
 * @retval NULL     Failed to create new TLS context.
 */
SSL_CTX *
smtp_test_seam_ssl_ctx_new(const SSL_METHOD *method){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_ctx_new_ctr)){
    return NULL;
  }
  return SSL_CTX_new(method);
}

/**
 * Allows the test harness to control when SSL_do_handshake() fails.
 *
 * @param[in] ssl OpenSSL handle.
 * @retval  1 TLS handshake successful.
 * @retval <1 TLS handshake failed.
 */
int
smtp_test_seam_ssl_do_handshake(SSL *ssl){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_do_handshake_ctr)){
    return 0;
  }
  return SSL_do_handshake(ssl);
}

/**
 * Allows the test harness to control when SSL_get_peer_certificate() fails.
 *
 * @param[in] ssl OpenSSL handle.
 * @retval X509* Peer certficate which must get freed by using X509_free().
 * @retval NULL Failed to get the peer certificate.
 */
X509 *
smtp_test_seam_ssl_get_peer_certificate(const SSL *ssl){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_get_peer_certificate_ctr)){
    return NULL;
  }
  return SSL_get_peer_certificate(ssl);
}

/**
 * Allows the test harness to control when X509_check_host() fails.
 *
 * @param[in] cert     X509 certificate handle.
 * @param[in] name     Server name.
 * @param[in] namelen  Number of characters in @p name or 0 if null-terminated.
 * @param[in] flags    Usually set to 0.
 * @param[in] peername Pointer to CN from certificate stored in this buffer
 *                     if not NULL.
 * @retval  1 Successful host check.
 * @retval  0 Failed host check.
 * @retval -1 Internal error.
 */
int
smtp_test_seam_x509_check_host(X509 *cert,
                               const char *name,
                               size_t namelen,
                               unsigned int flags,
                               char **peername){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_x509_check_host_ctr)){
    return -1;
  }
  return X509_check_host(cert, name, namelen, flags, peername);
}

/**
 * Allows the test harness to control when SSL_new() fails.
 *
 * @param[in] ctx OpenSSL TLS context.
 * @retval SSL* Pointer to a new TLS context.
 * @retval NULL Failed to create new TLS context.
 */
SSL *
smtp_test_seam_ssl_new(SSL_CTX *ctx){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_new_ctr)){
    return NULL;
  }
  return SSL_new(ctx);
}

/**
 * Allows the test harness to control when SSL_read() fails.
 *
 * @param[in] ssl OpenSSL TLS object.
 * @param[in] buf Store received data in this buffer.
 * @param[in] num Number of bytes in @p buf.
 * @retval  >0 Number of bytes successfully read from the TLS connection.
 * @retval <=0 Failed to read bytes on the TLS connection.
 */
int
smtp_test_seam_ssl_read(SSL *ssl,
                        void *buf,
                        int num){
  size_t bytes_inject_len;

  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_read_ctr)){
    if(*g_smtp_test_err_recv_bytes){
      bytes_inject_len = strlen(g_smtp_test_err_recv_bytes);
      assert(bytes_inject_len < (size_t)num && bytes_inject_len < INT_MAX);
      memcpy(buf, g_smtp_test_err_recv_bytes, bytes_inject_len);
      return (int)bytes_inject_len;
    }
    return -1;
  }
  return SSL_read(ssl, buf, num);
}

/**
 * Allows the test harness to control when SSL_write() fails.
 *
 * @param[in] ssl OpenSSL TLS object.
 * @param[in] buf Data to write to the TLS connection.
 * @param[in] num Number of bytes in @p buf.
 * @retval  >0 Number of bytes successfully written to the TLS connection.
 * @retval <=0 Failed to write bytes to the TLS connection.
 */
int
smtp_test_seam_ssl_write(SSL *ssl,
                         const void *buf,
                         int num){
  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_ssl_write_ctr)){
    return -1;
  }
  return SSL_write(ssl, buf, num);
}

/**
 * Allows the test harness to control when sprintf() fails.
 *
 * @param[in] s      Buffer to store the output contents to.
 * @param[in] format Format string defined in sprintf().
 * @retval >=0 Number of bytes copied to @p s, excluding the null-terminator.
 * @retval <0  Output or formatting error.
 */
int
smtp_test_seam_sprintf(char *s,
                       const char *format, ...){
  va_list ap;
  int rc;

  if(smtp_test_seam_dec_err_ctr(&g_smtp_test_err_sprintf_ctr)){
    errno = ENOMEM;
    return g_smtp_test_err_sprintf_rc;
  }
  va_start(ap, format);
  rc = vsprintf(s, format, ap);
  va_end(ap);
  return rc;
}

/**
 * Allows the test harness to control the return value of strlen().
 *
 * @param[in] s Null-terminated string.
 * @return Length of @p s.
 */
size_t
smtp_test_seam_strlen(const char *s){
  size_t result;

  if(g_smtp_test_strlen_custom_ret){
    result = g_smtp_test_strlen_ret_value;
  }
  else{
    result = strlen(s);
  }
  return result;
}

/**
 * Allows the test harness to control when time() fails.
 *
 * @param[out] tloc Buffer to hold the time_t results.
 * @retval >=0 Time in seconds since the Epoch.
 * @retval  -1 Failed to store the time in @p tloc.
 */
time_t
smtp_test_seam_time(time_t *tloc){
  if(g_smtp_test_time_custom_ret){
    return g_smtp_test_time_ret_value;
  }
  return time(tloc);
}

