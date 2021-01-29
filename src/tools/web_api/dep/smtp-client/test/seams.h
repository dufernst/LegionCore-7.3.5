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
#ifndef SMTP_TEST_SEAMS_H
#define SMTP_TEST_SEAMS_H

#include "test.h"

/*
 * Redefine these functions to internal test seam functions.
 */
#undef BIO_new_socket
#undef BIO_should_retry
#undef calloc
#undef close
#undef connect
#undef ERR_peek_error
#undef fclose
#undef ferror
#undef gmtime_r
#undef HMAC
#undef localtime_r
#undef malloc
#undef mktime
#undef realloc
#undef recv
#undef select
#undef send
#undef socket
#undef SSL_connect
#undef SSL_CTX_new
#undef SSL_do_handshake
#undef SSL_get_peer_certificate
#undef X509_check_host
#undef SSL_new
#undef SSL_read
#undef SSL_write
#undef sprintf
#undef strlen
#undef time

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_bio_new_socket.
 */
#define BIO_new_socket           smtp_test_seam_bio_new_socket

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_bio_should_retry.
 */
#define BIO_should_retry         smtp_test_seam_bio_should_retry

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_calloc.
 */
#define calloc                   smtp_test_seam_calloc

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_close.
 */
#define close                    smtp_test_seam_close

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_connect.
 */
#define connect                  smtp_test_seam_connect

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_err_peek_error.
 */
#define ERR_peek_error           smtp_test_seam_err_peek_error

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_fclose.
 */
#define fclose                   smtp_test_seam_fclose

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ferror.
 */
#define ferror                   smtp_test_seam_ferror

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_gmtime_r.
 */
#define gmtime_r                 smtp_test_seam_gmtime_r

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_hmac.
 */
#define HMAC                     smtp_test_seam_hmac

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_localtime_r.
 */
#define localtime_r              smtp_test_seam_localtime_r

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_malloc.
 */
#define malloc                   smtp_test_seam_malloc

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_mktime.
 */
#define mktime                   smtp_test_seam_mktime

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_realloc.
 */
#define realloc                  smtp_test_seam_realloc

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_recv.
 */
#define recv                     smtp_test_seam_recv

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_select.
 */
#define select                   smtp_test_seam_select

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_send.
 */
#define send                     smtp_test_seam_send

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_socket.
 */
#define socket                   smtp_test_seam_socket

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_connect.
 */
#define SSL_connect              smtp_test_seam_ssl_connect

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_ctx_new.
 */
#define SSL_CTX_new              smtp_test_seam_ssl_ctx_new

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_do_handshake.
 */
#define SSL_do_handshake         smtp_test_seam_ssl_do_handshake

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_get_peer_certificate.
 */
#define SSL_get_peer_certificate smtp_test_seam_ssl_get_peer_certificate

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_x509_check_host.
 */
#define X509_check_host smtp_test_seam_x509_check_host

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_new.
 */
#define SSL_new                  smtp_test_seam_ssl_new

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_read.
 */
#define SSL_read                 smtp_test_seam_ssl_read

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_ssl_write.
 */
#define SSL_write                smtp_test_seam_ssl_write

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_sprintf.
 */
#define sprintf                  smtp_test_seam_sprintf

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control the return value of this function.
 *
 * See @ref smtp_test_seam_strlen.
 */
#define strlen                   smtp_test_seam_strlen

/**
 * Redefine this function from smtp.c and inject a test seam which
 * can control when this function fails.
 *
 * See @ref smtp_test_seam_time.
 */
#define time                     smtp_test_seam_time

#endif

