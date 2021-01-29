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
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Get access to the @ref smtp_result_code and @ref smtp_command definitions.
 */
#define SMTP_INTERNAL_DEFINE

#include "test.h"

/**
 * Temporary file path to use for testing the @ref smtp_file_get_contents
 * function.
 */
#define TMP_FILE_PATH "/tmp/test_smtp_file_get_contents.txt"

/**
 * Maximum email subject buffer length.
 */
#define SMTP_TEST_SUBJECT_LEN        100

/**
 * Maximum email body buffer length.
 */
#define SMTP_TEST_BODY_LEN           1000

/**
 * Maximum server name buffer length.
 */
#define SMTP_MAX_SERVER_LEN          255

/**
 * Maximum certificate path length.
 */
#define SMTP_MAX_CAFILE_PATH         255

/**
 * Maximum server port buffer length.
 */
#define SMTP_MAX_PORT_LEN            10

/**
 * Maximum email buffer length.
 */
#define SMTP_MAX_EMAIL_LEN           255

/**
 * Maximum account password buffer length.
 */
#define SMTP_MAX_PASS_LEN            255

/**
 * Maximum file attachment name buffer length.
 */
#define SMTP_MAX_ATTACHMENT_NAME_LEN 100

/**
 * This default connection security method will get used for most test
 * connections with the SMTP server.
 */
#define SMTP_TEST_DEFAULT_CONNECTION_SECURITY SMTP_SECURITY_NONE

/**
 * This default authentication method will get used for most test connections
 * with the SMTP server.
 */
#define SMTP_TEST_DEFAULT_AUTH_METHOD         SMTP_AUTH_PLAIN

/**
 * These default flags will get used for most test connections with the
 * SMTP server.
 */
#define SMTP_TEST_DEFAULT_FLAGS               (enum smtp_flag)(  \
                                              SMTP_DEBUG |       \
                                              SMTP_NO_CERT_VERIFY)

/**
 * Use the default certificate path for OpenSSL.
 */
#define SMTP_TEST_DEFAULT_CAFILE              NULL

/**
 * Default name used by the address in the From: header.
 */
#define SMTP_TEST_DEFAULT_FROM_NAME           "From Name"

/**
 * Default name used by all addresses in the To: header.
 */
#define SMTP_TEST_DEFAULT_TO_NAME             "To Name"

/**
 * Default name used by all addresses in the Cc: header.
 */
#define SMTP_TEST_DEFAULT_CC_NAME             "Cc Name"

/**
 * Default name used by all BCC addresses.
 */
#define SMTP_TEST_DEFAULT_BCC_NAME            "Bcc Name"

/**
 * Some unit tests use this for testing encoding or splitting long strings.
 */
#define STR_ALPHABET_LOWERCASE "abcdefghijklmnopqrstuvwxyz"

/**
 * Stores details from the server configuration file.
 *
 * Most of these fields get loaded from separate configuration files because
 * those files contain sensitive account information.
 */
struct smtp_test_config{
  /**
   * SMTP client context.
   */
  struct smtp *smtp;

  /**
   * Email subject line.
   */
  char subject[SMTP_TEST_SUBJECT_LEN];

  /**
   * Email body text.
   */
  char body[SMTP_TEST_BODY_LEN];

  /*
   * The following fields get set by the configuration file.
   */

  /**
   * Server name or IP address.
   */
  char server[SMTP_MAX_SERVER_LEN];

  /**
   * Path to server certificate file.
   */
  char cafile[SMTP_MAX_CAFILE_PATH];

  /**
   * Server port number.
   */
  char port[SMTP_MAX_PORT_LEN];

  /**
   * Server port number for direct TLS connection.
   */
  char port_tls[SMTP_MAX_PORT_LEN];

  /**
   * Account user name.
   */
  char user[SMTP_MAX_EMAIL_LEN];

  /**
   * Account password.
   */
  char pass[SMTP_MAX_PASS_LEN];

  /**
   * The email displayed in the email from section.
   */
  char email_from[SMTP_MAX_EMAIL_LEN];

  /**
   * Email destination address.
   */
  char email_to[SMTP_MAX_EMAIL_LEN];

  /**
   * Email destination address (2nd).
   */
  char email_to_2[SMTP_MAX_EMAIL_LEN];

  /**
   * Email destination address (3rd).
   */
  char email_to_3[SMTP_MAX_EMAIL_LEN];
};

/**
 * Convenience global that allows test harnesses to easily check for return
 * code of smtp function calls.
 */
static enum smtp_status_code
g_rc;

/**
 * Convenience global that allows test harnesses to access configuration data
 * for the test email server.
 */
static struct smtp_test_config
g_config;

/**
 * Copy a string and guarantee that the destination string has been
 * null-terminated based on the given size.
 *
 * This function has a safer interface than strncpy because it always null
 * terminates the destination string and it returns the total number of bytes
 * in @p src which makes it easier to determine if the src tried to overflow
 * the buffer.
 *
 * @param[out] dest   Destination string buffer.
 * @param[in]  src    Source string buffer.
 * @param[in]  destsz Number of bytes available in @p dest.
 * @return String length of @p src.
 */
static size_t
smtp_strlcpy(char *dest,
             const char *src,
             size_t destsz){
  size_t src_idx;
  int found_end;

  found_end = 0;

  src_idx = 0;
  while(*src){
    if(!found_end){
      if(src_idx >= destsz - 1 || destsz == 0){
        dest[src_idx] = '\0';
        found_end = 1;
      }
      else{
        dest[src_idx] = *src;
      }
    }
    src_idx += 1;
    src += 1;
  }
  if(!found_end){
    dest[src_idx] = '\0';
  }

  return src_idx;
}

/**
 * Duplicate a string only up to a maximum number of bytes.
 *
 * @param[in] s String to duplicate.
 * @param[in] n Maximum number of bytes to copy.
 * @retval char* Duplicate of string @p s with at most @p n bytes.
 * @retval NULL  Memory allocation failure.
 */
static char *
smtp_strndup(const char *s,
             size_t n){
  char *ns;
  size_t newsz;

  newsz = sizeof(*ns) * (n + 1);
  ns = malloc(newsz);
  if(ns){
    smtp_strlcpy(ns, s, newsz);
  }

  return ns;
}

/**
 * Repeat a string multiple times and copy into new buffer.
 *
 * Used to test large inputs.
 *
 * @param[in] s String to repeat.
 * @param[in] n Number of times to repeat @p s.
 * @retval char* New buffer with text repeating @p s @p n times.
 * @retval NULL  Memory allocation failure.
 */
static char *
smtp_str_repeat(const char *const s,
                size_t n){
  char *snew;
  size_t slen;
  size_t snewlen;
  size_t i;

  slen = strlen(s);

  if(n < 1 || slen < 1){
    return smtp_strdup("");
  }

  snewlen = slen * n + 1;
  if((snew = malloc(snewlen)) == NULL){
    return NULL;
  }

  for(i = 0; i < n; i++){
    memcpy(&snew[slen * i], s, slen);
  }
  snew[snewlen - 1] = '\0';

  return snew;
}

/**
 * Holds a list of strings.
 *
 * Used by a number of utility functions below to store and operate on lists
 * of strings.
 */
struct smtp_str_list{
  /**
   * Number of strings in @p slist.
   */
  size_t n;

  /**
   * List of strings.
   */
  char **slist;
};

/**
 * Append a string to the string list.
 *
 * @param[in] slist String list to append to.
 * @param[in] s     The new string to append to the list.
 * @param[in] n     Maximum number of bytes to copy in the string.
 * @retval  0 Successfully appended the string to the list.
 * @retval -1 Memory allocation failure.
 */
static int
smtp_str_list_append(struct smtp_str_list *const slist,
                     const char *const s,
                     size_t n){
  char **slist_alloc;
  char *snew;

  if((slist_alloc = realloc(slist->slist,
                            sizeof(*slist->slist) * (slist->n + 1))) == NULL){
    return -1;
  }
  slist->slist = slist_alloc;

  if((snew = smtp_strndup(s, n)) == NULL){
    return -1;
  }
  slist->slist[slist->n] = snew;
  slist->n += 1;
  return 0;
}

/**
 * Free all memory associated to the string list.
 *
 * @param[in] list The string list to free.
 */
static void
smtp_str_list_free(struct smtp_str_list *const list){
  size_t i;

  for(i = 0; i < list->n; i++){
    free(list->slist[i]);
  }
  free(list->slist);
  list->slist = NULL;
  list->n = 0;
}

/**
 * Split a string with delimiters into a list.
 *
 * @param[in]  s         The string to split.
 * @param[in]  slen      Length of string @p s to split, or -1 to split the
 *                       entire string.
 * @param[in]  delimiter Split the string at every delimiter location.
 * @param[in]  limit     A positive limit will limit the maximum number of
 *                       split strings to @p limit with the last string
 *                       containing the rest of the string. A value of 0 has
 *                       the same meaning as 1. A negative value will cut off
 *                       the last @p limit strings from the result.
 * @param[out] slist     See @ref smtp_str_list.
 * @retval  0 Successfully split the string and stored the results into
 *            @p slist.
 * @retval -1 Memory allocation failure.
 */
static int
smtp_str_split(const char *const s,
               size_t slen,
               const char *const delimiter,
               int limit,
               struct smtp_str_list *slist){
  size_t i;
  size_t i1;
  size_t i2;
  size_t delimiter_len;
  int split_idx;

  memset(slist, 0, sizeof(*slist));
  delimiter_len = strlen(delimiter);

  if(slen == SIZE_MAX){
    slen = strlen(s);
  }

  split_idx = 0;

  i1 = 0;
  for(i2 = 0; i2 < slen; i2++){
    if(limit > 0 && limit - 1 <= split_idx){
      if(smtp_str_list_append(slist, &s[i1], SIZE_MAX) < 0){
        smtp_str_list_free(slist);
        return -1;
      }
      return 0;
    }
    else if(strncmp(&s[i2], delimiter, delimiter_len) == 0){
      if(i2 - i1 == 0 && s[i2] == '\0'){
        break;
      }
      if(smtp_str_list_append(slist, &s[i1], i2 - i1) < 0){
        smtp_str_list_free(slist);
        return -1;
      }
      i1 = i2 + delimiter_len;
      i2 = i1;
      if(strncmp(&s[i2], delimiter, delimiter_len) == 0){
        i2 -= 1;
      }
      split_idx += 1;
    }
  }

  if(smtp_str_list_append(slist, &s[i1], i2 - i1) < 0){
    smtp_str_list_free(slist);
    return -1;
  }

  if(limit < 0){
    for(i = 0; i < (size_t)abs(limit); i++){
      free(slist->slist[slist->n - i - 1]);
    }
    slist->n -= i;
  }

  return 0;
}

/**
 * Write bytes to an open file stream.
 *
 * @param[in] stream The file stream to write bytes to.
 * @param[in] data   The buffer containing the contents to write to the
 *                   file stream.
 * @param[in] datasz Number of bytes in @p data.
 * @return Number of bytes written to the file.
 */
static size_t
smtp_ffile_put_contents(FILE *stream,
                        const void *const data,
                        size_t datasz){
  size_t bytes_written;

  bytes_written = 0;

  if(datasz == SIZE_MAX){
    bytes_written = strlen(data);
    if(fputs(data, stream) == EOF){
      bytes_written = 0;
    }
  }
  else{
    bytes_written = fwrite(data, 1, datasz, stream);
  }

  return bytes_written;
}

/**
 * Write a byte string to a file.
 *
 * This interface handles safely opening, writing, and closing the file.
 *
 * A return of 0 can either indicate an error or it could indicate that the
 * string did not have any bytes to write. Check errno for if further details
 * required.
 *
 * @param[in] filename Path to file for writing the contents.
 * @param[in] data     The buffer contents to write to file.
 * @param[in] datasz   Number of bytes in @p data, or -1 if @p data consists
 *                     of a null-terminated string.
 * @param[in] flags    Set to 0 for write mode or O_APPEND for append mode.
 * @retval  0 Failed to write any bytes to the file.
 * @retval >0 Number of bytes written to file.
 */
static size_t
smtp_file_put_contents(const char *const filename,
                       const void *const data,
                       size_t datasz,
                       int flags){
  FILE *fp;
  size_t bytes_written;
  const char *mode;

  if(flags == 0){
    mode = "w";
  }
  else if(flags == O_APPEND){
    mode = "a";
  }
  else{
    errno = EINVAL;
    return 0;
  }

  if((fp = fopen(filename, mode)) == NULL){
    return 0;
  }

  bytes_written = smtp_ffile_put_contents(fp, data, datasz);

  if(fclose(fp) == EOF){
    return 0;
  }

  return bytes_written;
}

/**
 * Sleep for number of seconds.
 *
 * Useful for testing failure scenarios because a timeout will occur after
 * too many failed login attempts.
 *
 * @param[in] seconds The number of seconds to pause execution.
 */
static void
smtp_test_sleep(unsigned int seconds){
  fprintf(stderr, "TEST FRAMEWORK: sleeping for %u seconds\n", seconds);
  assert(sleep(seconds) == 0);
}

/**
 * Test harness for @ref smtp_si_add_size_t, @ref smtp_si_sub_size_t,
 * and @ref smtp_si_mul_size_t.
 *
 * @param[in]  si_fp         One of the si wrapping functions for size_t types.
 * @param[in]  a             Add this value with @p b.
 * @param[in]  b             Add this value with @p a.
 * @param[out] result        Memory location to store the result.
 * @param[in]  expect_result Expected result of adding @p a and @p b.
 * @param[in]  expect_wrap   Expectation on whether the addition will wrap.
 */
static void
smtp_unit_test_si_size_t(int (*si_fp)(const size_t a,
                                      const size_t b,
                                      size_t *const result),
                         size_t a,
                         size_t b,
                         size_t *result,
                         size_t expect_result,
                         int expect_wrap){
  int wraps;

  wraps = si_fp(a, b, result);
  assert(wraps == expect_wrap);
  if(result){
    assert(*result == expect_result);
  }
}

/**
 * Run all test cases for integer wrapping.
 */
static void
smtp_unit_test_all_si(void){
  size_t result;

  /* add */
  smtp_unit_test_si_size_t(smtp_si_add_size_t, 0, 0, &result, 0, 0);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, 0, 1, &result, 1, 0);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, 1, 0, &result, 1, 0);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, 1, 1, &result, 2, 0);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, 1, 1, NULL, SIZE_MAX, 0);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, SIZE_MAX, 1, NULL, SIZE_MAX, 1);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, SIZE_MAX, 0, &result, SIZE_MAX, 0);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, SIZE_MAX, 1, &result, 0, 1);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, SIZE_MAX - 1, 2, &result, 0, 1);
  smtp_unit_test_si_size_t(smtp_si_add_size_t, SIZE_MAX, 2, &result, 1, 1);
  smtp_unit_test_si_size_t(smtp_si_add_size_t,
                           SIZE_MAX / 2,
                           SIZE_MAX / 2,
                           &result,
                           SIZE_MAX - 1,
                           0);

  /* subtraction */
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 0, 0, &result, 0, 0);
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 0, 1, &result, SIZE_MAX, 1);
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 1, 0, &result, 1, 0);
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 1, 1, &result, 0, 0);
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 2, 3, &result, SIZE_MAX, 1);
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 2, 1, NULL, SIZE_MAX, 0);
  smtp_unit_test_si_size_t(smtp_si_sub_size_t, 1, 2, NULL, SIZE_MAX, 1);

  /* multiply */
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, 0, 0, &result, 0, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, 0, 1, &result, 0, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, 1, 0, &result, 0, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, 1, 1, &result, 1, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, 1, 1, NULL, SIZE_MAX, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, SIZE_MAX, 2, NULL, SIZE_MAX, 1);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, 100, 12, &result, 1200, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t, SIZE_MAX, 1, &result, SIZE_MAX, 0);
  smtp_unit_test_si_size_t(smtp_si_mul_size_t,
                           SIZE_MAX,
                           2,
                           &result,
                           SIZE_MAX * 2,
                           1);
}

/**
 * Test harness for @ref smtp_base64_decode.
 *
 * @param[in] buf            Null-terminated base64 string.
 * @param[in] expect_str     Decoded binary data.
 * @param[in] expect_str_len Length of @p expect_str.
 */
static void
smtp_unit_test_base64_decode(const char *const buf,
                             const char *const expect_str,
                             size_t expect_str_len){
  unsigned char *decode;
  size_t str_len;

  str_len = smtp_base64_decode(buf, &decode);
  if(expect_str){
    assert(memcmp(decode, expect_str, str_len) == 0);
    free(decode);
  }
  else{ /* NULL */
    assert(decode == NULL);
  }
  assert(str_len == expect_str_len);
}

/**
 * Run all test cases for base64 decoding.
 */
static void
smtp_unit_test_all_base64_decode(void){
  smtp_unit_test_base64_decode(""        , "", 0);
  smtp_unit_test_base64_decode("YQ=="    , "a", 1);

  smtp_unit_test_base64_decode("YWE="    , "aa"   , 2);
  smtp_unit_test_base64_decode("YWFh"    , "aaa"  , 3);
  smtp_unit_test_base64_decode("YWFhYQ==", "aaaa" , 4);
  smtp_unit_test_base64_decode("YWFhYWE=", "aaaaa", 5);
  smtp_unit_test_base64_decode("MTIzNDU=", "12345", 5);
  smtp_unit_test_base64_decode("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=",
                               STR_ALPHABET_LOWERCASE,
                               26);

  /* invalid inputs */
  smtp_unit_test_base64_decode("AB"     , NULL, SIZE_MAX);
  smtp_unit_test_base64_decode("^^^^"   , NULL, SIZE_MAX);
  smtp_unit_test_base64_decode("^^^\xFF", NULL, SIZE_MAX);

  /* Wrap when calculating the buffer length. */
  g_smtp_test_err_si_add_size_t_ctr = 0;
  smtp_unit_test_base64_decode("YQ=="    , NULL, SIZE_MAX);
  g_smtp_test_err_si_add_size_t_ctr = -1;

  g_smtp_test_err_calloc_ctr = 0;
  smtp_unit_test_base64_decode("", NULL, SIZE_MAX);
  g_smtp_test_err_calloc_ctr = -1;
}

/**
 * Test harness for @ref smtp_base64_encode.
 *
 * @param[in] buf    Binary data to encode in base64.
 * @param[in] buflen Number of bytes in the @p buf parameter.
 * @param[in] expect The expected base64 string that would get returned.
 */
static void
smtp_unit_test_base64_encode(const char *const buf,
                             size_t buflen,
                             const char *const expect){
  char *result;

  result = smtp_base64_encode(buf, buflen);
  if(expect){
    assert(strcmp(result, expect) == 0);
    free(result);
  }
  else{ /* NULL */
    assert(result == expect);
  }
}

/**
 * Run all test cases for base64 encoding.
 */
static void
smtp_unit_test_all_base64_encode(void){
  smtp_unit_test_base64_encode(""     , SIZE_MAX, "");
  smtp_unit_test_base64_encode("a"    , SIZE_MAX, "YQ==");
  smtp_unit_test_base64_encode("aa"   , SIZE_MAX, "YWE=");
  smtp_unit_test_base64_encode("aaa"  , SIZE_MAX, "YWFh");
  smtp_unit_test_base64_encode("aaaa" , SIZE_MAX, "YWFhYQ==");
  smtp_unit_test_base64_encode("aaaaa", SIZE_MAX, "YWFhYWE=");
  smtp_unit_test_base64_encode("12345", SIZE_MAX, "MTIzNDU=");
  smtp_unit_test_base64_encode(STR_ALPHABET_LOWERCASE,
                               SIZE_MAX,
                               "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=");

  /* binary data */
  smtp_unit_test_base64_encode("a\0b\1c", 5, "YQBiAWM=");
  smtp_unit_test_base64_encode("a\n\r\4bc", 6, "YQoNBGJj");

  /* Value would wrap */
  g_smtp_test_strlen_custom_ret = 1;
  g_smtp_test_strlen_ret_value = SIZE_MAX - 1;
  smtp_unit_test_base64_encode("a", SIZE_MAX, NULL);
  g_smtp_test_strlen_custom_ret = 0;
  g_smtp_test_strlen_ret_value = 0;

  /* calloc */
  g_smtp_test_err_calloc_ctr = 0;
  smtp_unit_test_base64_encode("", SIZE_MAX, NULL);
  g_smtp_test_err_calloc_ctr = -1;
}

/**
 * Test harness for @ref smtp_bin2hex.
 *
 * @param[in] s      Buffer containing binary data to convert.
 * @param[in] slen   Number of bytes in @p s.
 * @param[in] expect Expected hex string output returned by @ref smtp_bin2hex.
 */
static void
smtp_unit_test_bin2hex(const char *const s,
                       size_t slen,
                       const char *const expect){
  char *result;

  result = smtp_bin2hex((const unsigned char *const)s, slen);
  if(expect){
    assert(strcmp(result, expect) == 0);
  }
  else{
    assert(result == expect);
  }
  free(result);
}

/**
 * Run all test cases for @ref smtp_bin2hex.
 */
static void
smtp_unit_test_all_bin2hex(void){
  smtp_unit_test_bin2hex(""      , 0, ""            );
  smtp_unit_test_bin2hex("0"     , 0, ""            );
  smtp_unit_test_bin2hex("0"     , 1, "30"          );
  smtp_unit_test_bin2hex("1"     , 1, "31"          );
  smtp_unit_test_bin2hex("012345", 6, "303132333435");
  smtp_unit_test_bin2hex("012345", 3, "303132"      );
  smtp_unit_test_bin2hex("000000", 6, "303030303030");
  smtp_unit_test_bin2hex(
    STR_ALPHABET_LOWERCASE,
    26,
    "6162636465666768696a6b6c6d6e6f707172737475767778797a");
  smtp_unit_test_bin2hex("\xFF", 1, "ff");
  smtp_unit_test_bin2hex("\x00", 1, "00");

  /* Multiplication wrap. */
  smtp_unit_test_bin2hex("", SIZE_MAX - 2, NULL);

  /* Addition wrap. */
  g_smtp_test_err_si_add_size_t_ctr = 0;
  smtp_unit_test_bin2hex("", SIZE_MAX / 2, NULL);
  g_smtp_test_err_si_add_size_t_ctr = -1;

  g_smtp_test_err_malloc_ctr = 0;
  smtp_unit_test_bin2hex("", 0, NULL);
  g_smtp_test_err_malloc_ctr = -1;

  g_smtp_test_err_sprintf_ctr = 0;
  g_smtp_test_err_sprintf_rc = -1;
  smtp_unit_test_bin2hex("0", 1, NULL);
  g_smtp_test_err_sprintf_ctr = -1;

  g_smtp_test_err_sprintf_ctr = 0;
  g_smtp_test_err_sprintf_rc = 10;
  smtp_unit_test_bin2hex("0", 1, NULL);
  g_smtp_test_err_sprintf_ctr = -1;
}

/**
 * Test harness for @ref smtp_stpcpy.
 *
 * @param[in] init   Set the destination buffer to this initial string.
 * @param[in] s2     Concatenate this string into the destination buffer.
 * @param[in] expect Expected string result.
 */
static void
smtp_unit_test_stpcpy(const char *const init,
                      const char *const s2,
                      const char *const expect){
  char *buf;
  size_t bufsz;
  char *endptr;
  char *expect_ptr;

  bufsz = strlen(init) + strlen(s2) + 1;
  buf = malloc(bufsz);
  assert(buf);

  strcpy(buf, init);
  endptr = buf + strlen(init);

  endptr = smtp_stpcpy(endptr, s2);
  expect_ptr = buf + bufsz - 1;
  assert(endptr == expect_ptr);
  assert(*endptr == '\0');
  assert(strcmp(buf, expect) == 0);
  free(buf);
}

/**
 * Run all test cases for @ref smtp_stpcpy.
 */
static void
smtp_unit_test_all_stpcpy(void){
  smtp_unit_test_stpcpy("", "", "");
  smtp_unit_test_stpcpy("", "a", "a");
  smtp_unit_test_stpcpy("", "ab", "ab");
  smtp_unit_test_stpcpy("", "abc", "abc");

  smtp_unit_test_stpcpy("a", "", "a");
  smtp_unit_test_stpcpy("ab", "", "ab");
  smtp_unit_test_stpcpy("abc", "", "abc");

  smtp_unit_test_stpcpy("a", "a", "aa");
}

/**
 * Test harness for @ref smtp_reallocarray.
 *
 * @param[in] ptr          Existing allocation buffer, or NULL when allocating
 *                         a new buffer.
 * @param[in] nmemb        Number of elements to allocate.
 * @param[in] size         Size of each element in @p nmemb.
 * @param[in] expect_alloc Set to 0 if expecting NULL return, or set to any
 *                         other value when expecting successful allocation.
 */
static void
smtp_unit_test_reallocarray(void *ptr,
                            size_t nmemb,
                            size_t size,
                            int expect_alloc){
  void *result;

  result = smtp_reallocarray(ptr, nmemb, size);
  if(expect_alloc){
    assert(result);
    free(result);
  }
  else{
    assert(result == NULL);
  }
}

/**
 * Run all test cases for @ref smtp_reallocarray.
 */
static void
smtp_unit_test_all_reallocarray(void){
  smtp_unit_test_reallocarray(NULL, 1, 1, 1);
  smtp_unit_test_reallocarray(NULL, 0, 1, 1);
  smtp_unit_test_reallocarray(NULL, 1, 0, 1);

  /* unsigned wrapping */
  smtp_unit_test_reallocarray(NULL, SIZE_MAX, 2, 0);

  /* realloc */
  g_smtp_test_err_realloc_ctr = 0;
  smtp_unit_test_reallocarray(NULL, 1, 1, 0);
  g_smtp_test_err_realloc_ctr = -1;
}

/**
 * Test harness for @ref smtp_strdup.
 *
 * @param[in] s      String to duplicate.
 * @param[in] expect Expected string result.
 */
static void
smtp_unit_test_strdup(const char *const s,
                      const char *const expect){
  char *result;

  result = smtp_strdup(s);
  if(expect){
    assert(strcmp(result, expect) == 0);
    free(result);
  }
  else{ /* NULL */
    assert(result == expect);
  }
}

/**
 * Run all test cases for @ref smtp_strdup.
 */
static void
smtp_unit_test_all_strdup(void){
  smtp_unit_test_strdup("", "");
  smtp_unit_test_strdup("a", "a");
  smtp_unit_test_strdup("ab", "ab");

  /* Wrap when calculating the new strlen. */
  g_smtp_test_strlen_custom_ret = 1;
  g_smtp_test_strlen_ret_value = SIZE_MAX;
  smtp_unit_test_strdup("a", NULL);
  g_smtp_test_strlen_custom_ret = 0;
  g_smtp_test_strlen_ret_value = 0;

  /* malloc */
  g_smtp_test_err_malloc_ctr = 0;
  smtp_unit_test_strdup("", NULL);
  g_smtp_test_err_malloc_ctr = -1;
}

/**
 * Test harness for @ref smtp_str_replace.
 *
 * @param[in] search  Substring to search for in @p s.
 * @param[in] replace Replace each instance of the search string with this.
 * @param[in] s       Null-terminated string to search and replace.
 * @param[in] expect  Expected result.
 */
static void
smtp_unit_test_str_replace(const char *const search,
                           const char *const replace,
                           const char *const s,
                           const char *const expect){
  char *result;

  result = smtp_str_replace(search, replace, s);
  if(expect){
    assert(strcmp(result, expect) == 0);
    free(result);
  }
  else{ /* NULL */
    assert(result == expect);
  }
}

/**
 * Run all tests for @ref smtp_str_replace.
 */
static void
smtp_unit_test_all_str_replace(void){
  int i;

  smtp_unit_test_str_replace("", "", "", "");
  smtp_unit_test_str_replace("a", "b", "", "");
  smtp_unit_test_str_replace("", "", "a b c", "a b c");
  smtp_unit_test_str_replace("a", "", "a b c", " b c");
  smtp_unit_test_str_replace("a", "a", "a", "a");
  smtp_unit_test_str_replace("a", "b", "a", "b");
  smtp_unit_test_str_replace("a", "bc", "a", "bc");
  smtp_unit_test_str_replace("a", "b", "abc", "bbc");
  smtp_unit_test_str_replace("A", "b", "a", "a");
  smtp_unit_test_str_replace("b", "a", "abc", "aac");
  smtp_unit_test_str_replace("string", "test", "test string", "test test");
  smtp_unit_test_str_replace("a", "b", "a b a", "b b b");
  smtp_unit_test_str_replace("a", "b", "a b a", "b b b");
  smtp_unit_test_str_replace("a", "b", "a b a b a", "b b b b b");

  g_smtp_test_strlen_custom_ret = 1;
  g_smtp_test_strlen_ret_value = SIZE_MAX;
  smtp_unit_test_str_replace("a", "b", "a b c", NULL);
  g_smtp_test_strlen_custom_ret = 0;
  g_smtp_test_strlen_ret_value = 0;

  for(i = 0; i < 10; i++){
    g_smtp_test_err_si_add_size_t_ctr = i;
    smtp_unit_test_str_replace("a", "b", "a b c", NULL);
    g_smtp_test_err_si_add_size_t_ctr = -1;

    g_smtp_test_err_si_add_size_t_ctr = i;
    smtp_unit_test_str_replace("b", "a", "a b c", NULL);
    g_smtp_test_err_si_add_size_t_ctr = -1;
  }

  g_smtp_test_err_realloc_ctr = 0;
  smtp_unit_test_str_replace("a", "b", "a b c", NULL);
  g_smtp_test_err_realloc_ctr = -1;

  g_smtp_test_err_realloc_ctr = 0;
  smtp_unit_test_str_replace("b", "a", "a b c", NULL);
  g_smtp_test_err_realloc_ctr = -1;
}

/**
 * Run all tests for @ref smtp_utf8_charlen.
 */
static void
smtp_unit_test_all_smtp_utf8_charlen(void){
  const char *utf8_str;

  assert(smtp_utf8_charlen('a') == 1);

  utf8_str = "щ";
  assert(smtp_utf8_charlen(utf8_str[0]) == 2);

  utf8_str = "€";
  assert(smtp_utf8_charlen(utf8_str[0]) == 3);

  utf8_str = "𠜎";
  assert(smtp_utf8_charlen(utf8_str[0]) == 4);
}

/**
 * Run all tests for @ref smtp_str_has_nonascii_utf8.
 */
static void
smtp_unit_test_all_smtp_str_has_nonascii_utf8(void){
  assert(smtp_str_has_nonascii_utf8("") == 0);
  assert(smtp_str_has_nonascii_utf8("abc") == 0);
  assert(smtp_str_has_nonascii_utf8("?") == 0);
  assert(smtp_str_has_nonascii_utf8("щ") == 1);
  assert(smtp_str_has_nonascii_utf8("abщ") == 1);
}

/**
 * Test harness for @ref smtp_strnlen_utf8.
 *
 * @param[in] s      UTF-8 string.
 * @param[in] maxlen Do not check more than @p maxlen bytes of string @p s.
 * @param[in] expect Expected string length.
 */
static void
smtp_unit_test_strnlen_utf8(const char *s,
                            size_t maxlen,
                            size_t expect){
  size_t slen;

  slen = smtp_strnlen_utf8(s, maxlen);
  assert(slen == expect);
}

/**
 * Run all tests for @ref smtp_strnlen_utf8.
 */
static void
smtp_unit_test_all_strnlen_utf8(void){
  smtp_unit_test_strnlen_utf8(""  , 0, 0);
  smtp_unit_test_strnlen_utf8(""  , 1, 0);
  smtp_unit_test_strnlen_utf8("a" , 0, 0);
  smtp_unit_test_strnlen_utf8("a" , 1, 1);
  smtp_unit_test_strnlen_utf8("a" , 2, 1);
  smtp_unit_test_strnlen_utf8("ab", 0, 0);
  smtp_unit_test_strnlen_utf8("ab", 1, 1);
  smtp_unit_test_strnlen_utf8("ab", 2, 2);
  smtp_unit_test_strnlen_utf8("ab", 3, 2);

  smtp_unit_test_strnlen_utf8("щ", 0, 0);
  smtp_unit_test_strnlen_utf8("щ", 1, 2);
  smtp_unit_test_strnlen_utf8("щ", 2, 2);
  smtp_unit_test_strnlen_utf8("щ", 3, 2);

  smtp_unit_test_strnlen_utf8("€", 0, 0);
  smtp_unit_test_strnlen_utf8("€", 1, 3);
  smtp_unit_test_strnlen_utf8("€", 2, 3);
  smtp_unit_test_strnlen_utf8("€", 3, 3);

  smtp_unit_test_strnlen_utf8("€€", 0, 0);
  smtp_unit_test_strnlen_utf8("€€", 1, 3);
  smtp_unit_test_strnlen_utf8("€€", 2, 3);
  smtp_unit_test_strnlen_utf8("€€", 3, 3);
  smtp_unit_test_strnlen_utf8("€€", 4, 6);
  smtp_unit_test_strnlen_utf8("€€", 5, 6);
  smtp_unit_test_strnlen_utf8("€€", 6, 6);
  smtp_unit_test_strnlen_utf8("€€", 7, 6);

  smtp_unit_test_strnlen_utf8("𠜎", 0, 0);
  smtp_unit_test_strnlen_utf8("𠜎", 1, 4);
  smtp_unit_test_strnlen_utf8("𠜎", 2, 4);
  smtp_unit_test_strnlen_utf8("𠜎", 3, 4);

  /* Invalid UTF-8 sequences. */
  smtp_unit_test_strnlen_utf8("\xBF", 3, SIZE_MAX);
  smtp_unit_test_strnlen_utf8("\xC0", 3, SIZE_MAX);
}

/**
 * Test harness for @ref smtp_fold_whitespace_get_offset.
 *
 * @param[in] s      String to fold.
 * @param[in] maxlen Number of bytes for each line in the string (soft limit).
 * @param[in] expect Expected folded string.
 */
static void
smtp_unit_test_fold_whitespace_get_offset(const char *const s,
                                          unsigned int maxlen,
                                          size_t expect){
  size_t result;

  result = smtp_fold_whitespace_get_offset(s, maxlen);
  assert(result == expect);
}

/**
 * Run all tests for @ref smtp_fold_whitespace_get_offset.
 */
static void
smtp_unit_test_all_fold_whitespace_get_offset(void){
  smtp_unit_test_fold_whitespace_get_offset("", 0, 0);
  smtp_unit_test_fold_whitespace_get_offset("", 1, 0);
  smtp_unit_test_fold_whitespace_get_offset("", 2, 0);

  smtp_unit_test_fold_whitespace_get_offset("a", 0, 1);
  smtp_unit_test_fold_whitespace_get_offset("a", 1, 1);
  smtp_unit_test_fold_whitespace_get_offset("a", 2, 1);
  smtp_unit_test_fold_whitespace_get_offset("a", 3, 1);

  smtp_unit_test_fold_whitespace_get_offset("ab", 0, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab", 1, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab", 2, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab", 3, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab", 4, 2);

  smtp_unit_test_fold_whitespace_get_offset("abc", 0, 3);
  smtp_unit_test_fold_whitespace_get_offset("abc", 1, 3);
  smtp_unit_test_fold_whitespace_get_offset("abc", 2, 3);
  smtp_unit_test_fold_whitespace_get_offset("abc", 3, 3);
  smtp_unit_test_fold_whitespace_get_offset("abc", 4, 3);
  smtp_unit_test_fold_whitespace_get_offset("abc", 5, 3);

  smtp_unit_test_fold_whitespace_get_offset("a b", 0, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b", 1, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b", 2, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b", 3, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b", 4, 3);
  smtp_unit_test_fold_whitespace_get_offset("a b", 5, 3);

  smtp_unit_test_fold_whitespace_get_offset("a \tb", 0, 2);
  smtp_unit_test_fold_whitespace_get_offset("a \tb", 1, 2);
  smtp_unit_test_fold_whitespace_get_offset("a \tb", 2, 2);
  smtp_unit_test_fold_whitespace_get_offset("a \tb", 3, 2);
  smtp_unit_test_fold_whitespace_get_offset("a \tb", 4, 2);
  smtp_unit_test_fold_whitespace_get_offset("a \tb", 5, 4);
  smtp_unit_test_fold_whitespace_get_offset("a \tb", 6, 4);

  smtp_unit_test_fold_whitespace_get_offset("a b c", 0, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 1, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 2, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 3, 1);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 4, 3);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 5, 3);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 6, 5);
  smtp_unit_test_fold_whitespace_get_offset("a b c", 7, 5);

  smtp_unit_test_fold_whitespace_get_offset("ab c", 0, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab c", 1, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab c", 2, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab c", 3, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab c", 4, 2);
  smtp_unit_test_fold_whitespace_get_offset("ab c", 5, 4);
  smtp_unit_test_fold_whitespace_get_offset("ab c", 6, 4);

  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 0, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 1, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 2, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 3, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 4, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 5, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 6, 3);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 7, 6);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 8, 7);
  smtp_unit_test_fold_whitespace_get_offset(" ab cd ", 9, 7);

  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  0, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  1, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  2, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  3, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  4, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  5, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  6, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  7, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  8, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ",  9, 5);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ", 10, 9);
  smtp_unit_test_fold_whitespace_get_offset("\t ab\t cd\t ", 11, 10);

  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 0, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 1, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 2, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 8, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 9, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 10, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 10, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 10, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 10, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 10, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: Test Email WS", 10, 8);

  smtp_unit_test_fold_whitespace_get_offset("Subject:", 0, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 1, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 2, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 3, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 4, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 5, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 6, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 7, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 8, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 9, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject:", 10, 8);

  smtp_unit_test_fold_whitespace_get_offset("Subject: ", 0, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: ", 9, 8);
  smtp_unit_test_fold_whitespace_get_offset("Subject: ", 10, 9);
}

/**
 * Test harness for @ref smtp_fold_whitespace.
 *
 * @param[in] s      String to fold.
 * @param[in] maxlen Number of bytes for each line in the string (soft limit).
 * @param[in] expect Expected folded string.
 */
static void
smtp_unit_test_fold_whitespace(const char *const s,
                               unsigned int maxlen,
                               const char *const expect){
  char *result;

  result = smtp_fold_whitespace(s, maxlen);
  if(expect == NULL){
    assert(result == expect);
  }
  else{
    assert(strcmp(result, expect) == 0);
    free(result);
  }
}

/**
 * Run all tests for @ref smtp_fold_whitespace.
 */
static void
smtp_unit_test_all_fold_whitespace(void){
  int i;

  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 5,
                                 "Subject:\r\n"
                                 " Email\r\n"
                                 " Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 14,
                                 "Subject:\r\n"
                                 " Email Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 15,
                                 "Subject:\r\n"
                                 " Email Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 16,
                                 "Subject:\r\n"
                                 " Email Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 17,
                                 "Subject: Email\r\n"
                                 " Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 18,
                                 "Subject: Email\r\n"
                                 " Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 19,
                                 "Subject: Email\r\n"
                                 " Test");
  smtp_unit_test_fold_whitespace("Subject: Email Test",
                                 50,
                                 "Subject: Email Test");
  smtp_unit_test_fold_whitespace("",
                                 50,
                                 "");
  smtp_unit_test_fold_whitespace("Subject: Long Email Subject Line [123456789]",
                                 17,
                                 "Subject: Long\r\n"
                                 " Email Subject\r\n"
                                 " Line\r\n"
                                 " [123456789]");
  smtp_unit_test_fold_whitespace(STR_ALPHABET_LOWERCASE,
                                 10,
                                 STR_ALPHABET_LOWERCASE);

  for(i = 0; i < 3; i++){
    g_smtp_test_err_si_add_size_t_ctr = i;
    smtp_unit_test_fold_whitespace("a b c", 2, NULL);
    g_smtp_test_err_si_add_size_t_ctr = -1;
  }

  /* Memory allocation failure. */
  g_smtp_test_err_realloc_ctr = 0;
  smtp_unit_test_fold_whitespace("a b c", 2, NULL);
  g_smtp_test_err_realloc_ctr = -1;

  /* Memory allocation failure second loop. */
  g_smtp_test_err_realloc_ctr = 1;
  smtp_unit_test_fold_whitespace("Subject: Email Test", 2, NULL);
  g_smtp_test_err_realloc_ctr = -1;
}

/**
 * Test harness for @ref smtp_chunk_split.
 *
 * @param[in] s        The string to chunk.
 * @param[in] chunklen Number of bytes for each chunk in the string.
 * @param[in] end      Terminating string placed at the end of each chunk.
 * @param[in] expect   Expected chunk string.
 */
static void
smtp_unit_test_chunk_split(const char *const s,
                           size_t chunklen,
                           const char *const end,
                           const char *const expect){
  char *result;

  result = smtp_chunk_split(s, chunklen, end);
  if(expect == NULL){
    assert(result == expect);
  }
  else{
    assert(strcmp(result, expect) == 0);
    free(result);
  }
}

/**
 * Run all tests for @ref smtp_chunk_split.
 */
static void
smtp_unit_test_all_chunk_split(void){
  int i;

  smtp_unit_test_chunk_split("", 0, "", NULL);
  smtp_unit_test_chunk_split("a", 0, "a", NULL);
  smtp_unit_test_chunk_split("", 1, "", "");
  smtp_unit_test_chunk_split("", 1, "a", "a");
  smtp_unit_test_chunk_split("", 2, "a", "a");
  smtp_unit_test_chunk_split("a", 1, "", "a");
  smtp_unit_test_chunk_split("abc", 1, "-", "a-b-c-");
  smtp_unit_test_chunk_split("abc", 2, "-", "ab-c-");
  smtp_unit_test_chunk_split("abc", 3, "-", "abc-");
  smtp_unit_test_chunk_split("abcdefghijklmnop",
                             3,
                             "-",
                             "abc-def-ghi-jkl-mno-p-");
  smtp_unit_test_chunk_split("abc", 1, "-!@", "a-!@b-!@c-!@");
  smtp_unit_test_chunk_split("abcdefghijklmnop",
                             3,
                             "-!",
                             "abc-!def-!ghi-!jkl-!mno-!p-!");
  smtp_unit_test_chunk_split("abc", 1, "\r\n", "a\r\nb\r\nc\r\n");
  smtp_unit_test_chunk_split(STR_ALPHABET_LOWERCASE,
                             10,
                             "\r\n",
                             "abcdefghij\r\nklmnopqrst\r\nuvwxyz\r\n");

  /*
   * UTF-8 characters
   * щ - 2 bytes
   * € - 3 bytes
   * 𠜎 - 4 bytes
   */
  smtp_unit_test_chunk_split("€€€", 1, "\r\n", "€\r\n€\r\n€\r\n");
  smtp_unit_test_chunk_split("€€€€€", 1, "\r\n", "€\r\n€\r\n€\r\n€\r\n€\r\n");
  smtp_unit_test_chunk_split("a€c", 1, "-", "a-€-c-");
  smtp_unit_test_chunk_split("a€c", 2, "-", "a€-c-");
  smtp_unit_test_chunk_split("€€€", 3, "-", "€-€-€-");
  smtp_unit_test_chunk_split("щbc", 3, "-", "щb-c-");
  smtp_unit_test_chunk_split("щbc", 4, "-", "щbc-");
  smtp_unit_test_chunk_split("aщ€𠜎e", 2, "-", "aщ-€-𠜎-e-");
  smtp_unit_test_chunk_split("aщ€𠜎e", 4, "-", "aщ€-𠜎-e-");

  /* Wrapping. */
  g_smtp_test_err_si_mul_size_t_ctr = 0;
  smtp_unit_test_chunk_split("abc", 1, "-", NULL);
  g_smtp_test_err_si_mul_size_t_ctr = -1;
  for(i = 0; i < 3; i++){
    g_smtp_test_err_si_add_size_t_ctr = i;
    smtp_unit_test_chunk_split("abc", 1, "-", NULL);
    g_smtp_test_err_si_add_size_t_ctr = -1;
  }

  /* Memory allocation failure. */
  g_smtp_test_err_calloc_ctr = 0;
  smtp_unit_test_chunk_split("abc", 1, "-", NULL);
  g_smtp_test_err_calloc_ctr = -1;

  /* Invalid UTF-8 characters. */
  smtp_unit_test_chunk_split("\xBF", 1, "-", NULL);
  smtp_unit_test_chunk_split("\xC0", 1, "-", NULL);
}

/**
 * Test harness for @ref smtp_file_get_contents.
 *
 * @param[in] s      The string to write to the temp file before reading.
 * @param[in] nbytes Number of bytes in @p s.
 * @param[in] expect Expected string after reading the file.
 */
static void
smtp_unit_test_file_get_contents(const char *const s,
                                 size_t nbytes,
                                 const char *const expect){
  char *read_buf;
  size_t nbytes_rw;

  nbytes_rw = smtp_file_put_contents(TMP_FILE_PATH, s, nbytes, 0);
  assert(nbytes_rw == strlen(expect));

  read_buf = smtp_file_get_contents(TMP_FILE_PATH, &nbytes_rw);
  assert(read_buf);

  assert(memcmp(expect, read_buf, strlen(expect)) == 0);
  free(read_buf);
}

/**
 * Run all tests for @ref smtp_file_get_contents.
 */
static void
smtp_unit_test_all_file_get_contents(void){
  const char *test_str;

  smtp_unit_test_file_get_contents("", 0, "");

  test_str = "test";
  smtp_unit_test_file_get_contents(test_str, 0, "");
  smtp_unit_test_file_get_contents(test_str, strlen(test_str), test_str);

  test_str = "test\nnewline";
  smtp_unit_test_file_get_contents(test_str, strlen(test_str), test_str);

  test_str = "test";
  smtp_unit_test_file_get_contents(test_str, SIZE_MAX, test_str);

  test_str = STR_ALPHABET_LOWERCASE;
  smtp_unit_test_file_get_contents(test_str, strlen(test_str), test_str);

  /* smtp_file_get_contents - fopen */
  assert(smtp_file_get_contents("", NULL) == NULL);

  /* smtp_file_get_contents - fclose */
  g_smtp_test_err_fclose_ctr = 0;
  assert(smtp_file_get_contents(TMP_FILE_PATH, NULL) == NULL);
  g_smtp_test_err_fclose_ctr = -1;

  /* Wrap when increasing the buffer size. */
  g_smtp_test_err_si_add_size_t_ctr = 0;
  assert(smtp_file_get_contents(TMP_FILE_PATH, NULL) == NULL);
  g_smtp_test_err_si_add_size_t_ctr = -1;

  /* smtp_file_get_contents - realloc */
  g_smtp_test_err_realloc_ctr = 0;
  assert(smtp_file_get_contents(TMP_FILE_PATH, NULL) == NULL);
  g_smtp_test_err_realloc_ctr = -1;

  /* smtp_file_get_contents - ferror */
  g_smtp_test_err_ferror_ctr = 0;
  assert(smtp_file_get_contents(TMP_FILE_PATH, NULL) == NULL);
  g_smtp_test_err_ferror_ctr = -1;
}

/**
 * Test harness for @ref smtp_parse_cmd_line.
 *
 * @param[in] line        The server response line to parse.
 * @param[in] expect_code Expected server response code.
 * @param[in] expect_more Set to 1 if more lines will get returned or 0 if
 *                        no more lines.
 * @param[in] expect_text Expected text shown after the response code.
 */
static void
smtp_unit_test_parse_cmd_line(const char *const line,
                              enum smtp_result_code expect_code,
                              int expect_more,
                              const char *const expect_text){
  char *line_dup;
  struct smtp_command cmd;
  int rc;

  line_dup = smtp_strdup(line);
  assert(line_dup);

  rc = smtp_parse_cmd_line(line_dup, &cmd);
  assert(rc == expect_code);
  assert(cmd.code == expect_code);
  assert(cmd.more == expect_more);
  assert(strcmp(cmd.text, expect_text) == 0);

  free(line_dup);
}

/**
 * Run all tests for @ref smtp_parse_cmd_line.
 */
static void
smtp_unit_test_all_parse_cmd_line(void){
  smtp_unit_test_parse_cmd_line("",
                                SMTP_INTERNAL_ERROR,
                                0,
                                "");
  smtp_unit_test_parse_cmd_line("<5",
                                SMTP_INTERNAL_ERROR,
                                0,
                                "<5");
  smtp_unit_test_parse_cmd_line("bad text",
                                SMTP_INTERNAL_ERROR,
                                0,
                                "text");
  smtp_unit_test_parse_cmd_line("bad-text",
                                SMTP_INTERNAL_ERROR,
                                1,
                                "text");
  smtp_unit_test_parse_cmd_line("0x1 text",
                                SMTP_INTERNAL_ERROR,
                                0,
                                "text");
  smtp_unit_test_parse_cmd_line("-22 text",
                                SMTP_INTERNAL_ERROR,
                                0,
                                "text");
  smtp_unit_test_parse_cmd_line("220 ready",
                                SMTP_READY,
                                0,
                                "ready");
}

/**
 * Test harness for @ref smtp_date_rfc_2822.
 *
 * @param[in] t         Force the time() function to return this time_t value.
 * @param[in] expect    Expected date string.
 * @param[in] expect_rc Expected return code.
 */
static void
smtp_unit_test_date_rfc_2822(time_t t,
                             const char *const expect,
                             int expect_rc){
  char result[1000];
  int rc;

  g_smtp_test_time_custom_ret = 1;
  g_smtp_test_time_ret_value = t;

  setenv("TZ", "UTC", 1);
  rc = smtp_date_rfc_2822(result);
  assert(rc == expect_rc);
  if(expect_rc == 0){
    assert(strcmp(result, expect) == 0);
  }

  g_smtp_test_time_custom_ret = 0;
  g_smtp_test_time_ret_value = 0;
}

/**
 * Run all tests for @ref smtp_date_rfc_2822.
 */
static void
smtp_unit_test_all_date_rfc_2822(void){
  smtp_unit_test_date_rfc_2822(0, "Thu, 01 Jan 1970 00:00:00 +0000", 0);
  smtp_unit_test_date_rfc_2822(60 * 60 * 24 * 2 + 5,
                               "Sat, 03 Jan 1970 00:00:05 +0000",
                               0);

  smtp_unit_test_date_rfc_2822(-1, NULL, -1);

  g_smtp_test_err_localtime_r_ctr = 0;
  smtp_unit_test_date_rfc_2822(0, NULL, -1);
  g_smtp_test_err_localtime_r_ctr = -1;

  g_smtp_test_err_gmtime_r_ctr = 0;
  smtp_unit_test_date_rfc_2822(0, NULL, -1);
  g_smtp_test_err_gmtime_r_ctr = -1;

  g_smtp_test_err_mktime_ctr = 0;
  smtp_unit_test_date_rfc_2822(0, NULL, -1);
  g_smtp_test_err_mktime_ctr = -1;

  g_smtp_test_err_mktime_ctr = 1;
  smtp_unit_test_date_rfc_2822(0, NULL, -1);
  g_smtp_test_err_mktime_ctr = -1;

  g_smtp_test_err_sprintf_ctr = 0;
  g_smtp_test_err_sprintf_rc = -1;
  smtp_unit_test_date_rfc_2822(0, NULL, -1);
  g_smtp_test_err_sprintf_ctr = -1;
}

/**
 * Run all tests for @ref smtp_address_validate_email.
 */
static void
smtp_unit_test_all_smtp_address_validate_email(void){
  assert(smtp_address_validate_email(STR_ALPHABET_LOWERCASE) == 0);
  assert(smtp_address_validate_email("mail@example.com") == 0);
  assert(smtp_address_validate_email("īḑȋᵭ") == 0);
  assert(smtp_address_validate_email("<abc") == -1);
  assert(smtp_address_validate_email("abc>") == -1);
  assert(smtp_address_validate_email("\x7f") == -1);
  assert(smtp_address_validate_email("a b c") == -1);
}

/**
 * Run all tests for @ref smtp_address_validate_name.
 */
static void
smtp_unit_test_all_smtp_address_validate_name(void){
  assert(smtp_address_validate_name(STR_ALPHABET_LOWERCASE) == 0);
  assert(smtp_address_validate_name("John Doe") == 0);
  assert(smtp_address_validate_name("John O'Doe") == 0);
  assert(smtp_address_validate_name("īḑȋᵭ") == 0);
  assert(smtp_address_validate_name("a\nb\nc") == -1);
  assert(smtp_address_validate_name("\"abc") == -1);
  assert(smtp_address_validate_name("\x7f") == -1);
}

/**
 * Run all tests for @ref smtp_attachment_validate_name.
 */
static void
smtp_unit_test_all_smtp_attachment_validate_name(void){
  assert(smtp_attachment_validate_name(STR_ALPHABET_LOWERCASE) == 0);
  assert(smtp_attachment_validate_name("a b c") == 0);
  assert(smtp_attachment_validate_name("test.txt") == 0);
  assert(smtp_attachment_validate_name("īḑȋᵭ") == 0);
  assert(smtp_attachment_validate_name("a\nbc") == -1);
  assert(smtp_attachment_validate_name("\x7f") == -1);
  assert(smtp_attachment_validate_name("a\'bc") == -1);
  assert(smtp_attachment_validate_name("a\"bc") == -1);
}

/**
 * Run all tests for @ref smtp_header_key_validate.
 */
static void
smtp_unit_test_all_smtp_header_key_validate(void){
  assert(smtp_header_key_validate(STR_ALPHABET_LOWERCASE) == 0);
  assert(smtp_header_key_validate("") == -1);
  assert(smtp_header_key_validate("īḑȋᵭ") == -1);
  assert(smtp_header_key_validate("a b c") == -1);
  assert(smtp_header_key_validate("a\xff") == -1);
  assert(smtp_header_key_validate("a:b:c") == -1);
  assert(smtp_header_key_validate("a\nb\nc") == -1);
}

/**
 * Run all tests for @ref smtp_header_value_validate.
 */
static void
smtp_unit_test_all_smtp_header_value_validate(void){
  assert(smtp_header_value_validate(STR_ALPHABET_LOWERCASE) == 0);
  assert(smtp_header_value_validate("a\tb c") == 0);
  assert(smtp_header_value_validate("īḑȋᵭ") == 0);
  assert(smtp_header_value_validate("a\xff") == 0);
  assert(smtp_header_value_validate("a\nb\nc") == -1);
}

/**
 * Test harness for @ref smtp_status_code_errstr.
 *
 * @param[in] status_code See @ref smtp_status_code.
 * @param[in] expect      Expected error string.
 */
static void
smtp_unit_test_smtp_status_code_errstr(enum smtp_status_code status_code,
                                       const char *const expect){
  const char *result;

  result = smtp_status_code_errstr(status_code);
  assert(strcmp(result, expect) == 0);
}

/**
 * Run all tests for @ref smtp_status_code_errstr.
 */
static void
smtp_unit_test_all_smtp_status_code_errstr(void){
  smtp_unit_test_smtp_status_code_errstr(SMTP_STATUS_OK,
                                         "Success");
  smtp_unit_test_smtp_status_code_errstr(SMTP_STATUS_NOMEM,
                                         "Memory allocation failed");
  smtp_unit_test_smtp_status_code_errstr((enum smtp_status_code)-1,
                                         "Unknown error");
  smtp_unit_test_smtp_status_code_errstr((enum smtp_status_code)99,
                                         "Unknown error");
}

/**
 * User data pointer for testing the @ref str_getdelimfd interface.
 */
struct smtp_test_getdelimfd_fp{
  /**
   * Read from this file which should contain the contents used to test
   * the parser.
   */
  FILE *fp;
};

/**
 * Set to a non-zero value to force an error return value
 * in @ref smtp_unit_test_getdelimfd_fp.
 */
static int g_smtp_test_getdelimfd_fp_fail = 0;

/**
 * Read function used by the @ref smtp_str_getdelimfd interface.
 *
 * @param[in]  gdfd  See @ref str_getdelimfd.
 * @param[out] buf   Pointer to buffer for storing bytes read.
 * @param[in]  count Maximum number of bytes to try reading.
 * @retval >=0 Number of bytes read.
 * @retval -1  Failed to read from the socket.
 */
static long
smtp_unit_test_getdelimfd_fp(struct str_getdelimfd *const gdfd,
                             void *buf,
                             size_t count){
  struct smtp_test_getdelimfd_fp *getdelimfd_fp;
  size_t bytes_read;

  getdelimfd_fp = gdfd->user_data;
  bytes_read = fread(buf, sizeof(char), count, getdelimfd_fp->fp);
  if(g_smtp_test_getdelimfd_fp_fail){
    return -1;
  }
  return (long)bytes_read;
}

/**
 * Test harness for @ref smtp_str_getdelimfd.
 *
 * @param[in] input_string  Test string used in delimeter parsing.
 * @param[in] nbytes        Number of bytes in @p input_string.
 * @param[in] delim         Delimiter used to split the string.
 * @param[in] expect_rc     Expected return code.
 * @param[in] null_fp       If set, use a NULL read function pointer. Useful
                            for testing that error condition.
 * @param[in] expect_pieces Expected list of strings parsed from the file.
 */
static void
smtp_unit_test_str_getdelimfd(const char *const input_string,
                              size_t nbytes,
                              int delim,
                              enum str_getdelim_retcode expect_rc,
                              int null_fp,
                              const char *expect_pieces, ...){
  const char *piece;
  enum str_getdelim_retcode rc;
  size_t bytes_written;
  struct str_getdelimfd gdfd;
  struct smtp_test_getdelimfd_fp getdelimfd_fp;
  struct smtp_str_list slist;
  FILE *fp;
  size_t piece_idx;
  va_list ap;

  memset(&slist, 0, sizeof(slist));

  bytes_written = smtp_file_put_contents(TMP_FILE_PATH,
                                         input_string,
                                         nbytes,
                                         0);
  assert(bytes_written == nbytes);

  memset(&getdelimfd_fp, 0, sizeof(getdelimfd_fp));
  fp = fopen(TMP_FILE_PATH, "r");
  assert(fp);
  getdelimfd_fp.fp = fp;

  memset(&gdfd, 0, sizeof(gdfd));
  gdfd.delim           = delim;
  if(!null_fp){
    gdfd.getdelimfd_read = smtp_unit_test_getdelimfd_fp;
  }
  gdfd.user_data       = &getdelimfd_fp;

  do{
    rc = smtp_str_getdelimfd(&gdfd);
    if(expect_rc == STRING_GETDELIMFD_ERROR){
      assert(rc == expect_rc);
      smtp_str_list_free(&slist);
      return;
    }
    assert(rc != STRING_GETDELIMFD_ERROR);
    assert(smtp_str_list_append(&slist, gdfd.line, gdfd.line_len) == 0);
  } while (rc != STRING_GETDELIMFD_DONE);
  smtp_str_getdelimfd_free(&gdfd);
  assert(fclose(fp) == 0);

  piece_idx = 0;
  piece = expect_pieces;
  va_start(ap, expect_pieces);
  while (piece){
    assert(strcmp(piece, slist.slist[piece_idx]) == 0);
    piece_idx += 1;
    piece = va_arg(ap, const char *);
  }
  va_end(ap);
  assert(piece_idx == slist.n);

  smtp_str_list_free(&slist);
}

/**
 * Test harness for @ref smtp_str_getdelimfd_set_line_and_buf.
 *
 * @param[in] gdfd          See @ref str_getdelimfd.
 * @param[in] copy_len      Number of bytes to copy to the internal line buffer.
 * @param[in] expect_result Expected result code (0 or -1) from
 *                          @ref smtp_str_getdelimfd_set_line_and_buf.
 */
static void
smtp_unit_test_getdelimfd_set_line_and_buf(struct str_getdelimfd *const gdfd,
                                           size_t copy_len,
                                           int expect_result){
  int result;

  result = smtp_str_getdelimfd_set_line_and_buf(gdfd, copy_len);
  assert(result == expect_result);
}

/**
 * Run all tests for @ref smtp_str_getdelimfd.
 */
static void
smtp_unit_test_all_str_getdelimfd(void){
  const char *s;
  struct str_getdelimfd gdfd;
  char test_str[] = "test";
  int i;

  s = "";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "",
                                NULL);

  s = "a";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "a",
                                NULL);

  s = "\n";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "",
                                "",
                                NULL);

  s = "a\n";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "a",
                                "",
                                NULL);

  s = "\na";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "",
                                "a",
                                NULL);

  s = "test line 1";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "test line 1",
                                NULL);

  s = "test line 1\n";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "test line 1",
                                "",
                                NULL);

  s = "test line 1\ntest line 2";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "test line 1",
                                "test line 2",
                                NULL);

  s = "test line 1\ntest line 2\ntest line 3";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_DONE,
                                0,
                                "test line 1",
                                "test line 2",
                                "test line 3",
                                NULL);

  /* smtp_str_getdelimfd_set_line_and_buf - 2 */
  g_smtp_test_err_calloc_ctr = 0;
  s = "a";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_ERROR,
                                0,
                                NULL);
  g_smtp_test_err_calloc_ctr = -1;

  /* smtp_str_getdelimfd_set_line_and_buf - 2 */
  g_smtp_test_err_calloc_ctr = 0;
  s = "a\na";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_ERROR,
                                0,
                                NULL);
  g_smtp_test_err_calloc_ctr = -1;

  /* realloc */
  g_smtp_test_err_realloc_ctr = 0;
  s = "a";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_ERROR,
                                0,
                                NULL);
  g_smtp_test_err_realloc_ctr = -1;

  /* fread */
  g_smtp_test_getdelimfd_fp_fail = 1;
  s = "a";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_ERROR,
                                0,
                                NULL);
  g_smtp_test_getdelimfd_fp_fail = 0;

  /* getdelimfd_read unset */
  smtp_unit_test_str_getdelimfd("", 0, '\0', STRING_GETDELIMFD_ERROR, 1, NULL);

  /* Test unsigned wrapping. */
  g_smtp_test_err_si_sub_size_t_ctr = 0;
  s = "a";
  smtp_unit_test_str_getdelimfd(s,
                                strlen(s),
                                '\n',
                                STRING_GETDELIMFD_ERROR,
                                0,
                                NULL);
  g_smtp_test_err_si_sub_size_t_ctr = -1;

  for(i = 0; i < 5; i++){
    g_smtp_test_err_si_add_size_t_ctr = i;
    s = "a";
    smtp_unit_test_str_getdelimfd(s,
                                  strlen(s),
                                  '\n',
                                  STRING_GETDELIMFD_ERROR,
                                  0,
                                  NULL);
    g_smtp_test_err_si_add_size_t_ctr = -1;
  }

  memset(&gdfd, 0, sizeof(gdfd));
  gdfd._buf = test_str;
  smtp_unit_test_getdelimfd_set_line_and_buf(&gdfd, SIZE_MAX, -1);
  smtp_unit_test_getdelimfd_set_line_and_buf(&gdfd, SIZE_MAX - 1, -1);
  smtp_unit_test_getdelimfd_set_line_and_buf(&gdfd, SIZE_MAX - 2, -1);
  g_smtp_test_err_si_sub_size_t_ctr = 1;
  smtp_unit_test_getdelimfd_set_line_and_buf(&gdfd, strlen(test_str), -1);
  g_smtp_test_err_si_sub_size_t_ctr = -1;
}

/**
 * Unit test functions which do not require an SMTP client context.
 */
static void
smtp_unit_test_all(void){
  smtp_unit_test_all_si();
  smtp_unit_test_all_base64_decode();
  smtp_unit_test_all_base64_encode();
  smtp_unit_test_all_bin2hex();
  smtp_unit_test_all_stpcpy();
  smtp_unit_test_all_reallocarray();
  smtp_unit_test_all_strdup();
  smtp_unit_test_all_str_replace();
  smtp_unit_test_all_smtp_utf8_charlen();
  smtp_unit_test_all_smtp_str_has_nonascii_utf8();
  smtp_unit_test_all_strnlen_utf8();
  smtp_unit_test_all_fold_whitespace_get_offset();
  smtp_unit_test_all_fold_whitespace();
  smtp_unit_test_all_chunk_split();
  smtp_unit_test_all_file_get_contents();
  smtp_unit_test_all_parse_cmd_line();
  smtp_unit_test_all_date_rfc_2822();
  smtp_unit_test_all_smtp_address_validate_email();
  smtp_unit_test_all_smtp_address_validate_name();
  smtp_unit_test_all_smtp_attachment_validate_name();
  smtp_unit_test_all_smtp_header_key_validate();
  smtp_unit_test_all_smtp_header_value_validate();
  smtp_unit_test_all_smtp_status_code_errstr();
  smtp_unit_test_all_str_getdelimfd();
}

/**
 * Load server configuration file containing server connection information.
 *
 * The server connection information contains sensitive information, so we
 * need to load it in from a separate configuration file that does not get
 * saved in the repository. This stores the parsed results into a
 * @ref smtp_test_config data structure.
 *
 * @param[in]  config_path Path to the server configuration file.
 * @retval  0 Successfully parsed and saved the server configuration into the
 *            data structure.
 * @retval -1 Failed to read file or memory allocation failure.
 */
static int
smtp_test_config_load_from_file(const char *const config_path){
  char *config_data;
  char *config_data_new;
  size_t config_data_len;
  struct smtp_str_list line_list;
  char *line;
  int rc;
  size_t i;
  const char *key;
  const char *value;

  memset(&g_config, 0, sizeof(g_config));

  if((config_data = smtp_file_get_contents(config_path,
                                           &config_data_len)) == NULL){
    return -1;
  }

  /* add a null character at end of file data */
  if((config_data_new = realloc(config_data, config_data_len + 1)) == NULL){
    free(config_data);
    return -1;
  }
  config_data = config_data_new;
  config_data[config_data_len] = '\0';

  rc = smtp_str_split(config_data, config_data_len, "\n", 0, &line_list);
  assert(rc == 0);
  free(config_data);

  for(i = 0; i < line_list.n; i++){
    line = line_list.slist[i];
    if(line[0] == '#'){
      continue;
    }
    key = strtok(line, "=");
    if(key == NULL){
      continue;
    }

    value = strtok(NULL, "=");
    if(value == NULL){
      value = "";
    }

    if(strcmp(key, "server") == 0){
      smtp_strlcpy(g_config.server, value, sizeof(g_config.server));
    }
    else if(strcmp(key, "cafile") == 0){
      smtp_strlcpy(g_config.cafile, value, sizeof(g_config.cafile));
    }
    else if(strcmp(key, "port") == 0){
      smtp_strlcpy(g_config.port, value, sizeof(g_config.port));
    }
    else if(strcmp(key, "port_tls") == 0){
      smtp_strlcpy(g_config.port_tls, value, sizeof(g_config.port_tls));
    }
    else if(strcmp(key, "user") == 0){
      smtp_strlcpy(g_config.user, value, sizeof(g_config.user));
    }
    else if(strcmp(key, "pass") == 0){
      smtp_strlcpy(g_config.pass, value, sizeof(g_config.pass));
    }
    else if(strcmp(key, "email_from") == 0){
      smtp_strlcpy(g_config.email_from, value, sizeof(g_config.email_from));
    }
    else if(strcmp(key, "email_to") == 0){
      smtp_strlcpy(g_config.email_to, value, sizeof(g_config.email_to));
    }
    else if(strcmp(key, "email_to_2") == 0){
      smtp_strlcpy(g_config.email_to_2, value, sizeof(g_config.email_to_2));
    }
    else if(strcmp(key, "email_to_3") == 0){
      smtp_strlcpy(g_config.email_to_3, value, sizeof(g_config.email_to_3));
    }
  }
  smtp_str_list_free(&line_list);

  return 0;
}

/**
 * Call @ref smtp_auth and check the result.
 *
 * @param[in] auth_method   See @ref smtp_authentication_method
 * @param[in] user          Username.
 * @param[in] pass          Password.
 * @param[in] expect_status Expected return code from @ref smtp_auth.
 */
static void
smtp_auth_check(const enum smtp_authentication_method auth_method,
                const char *const user,
                const char *const pass,
                const enum smtp_status_code expect_status){
  g_rc = smtp_auth(g_config.smtp, auth_method, user, pass);
  assert(g_rc == expect_status);
}

/**
 * Call @ref smtp_address_add and check the result.
 *
 * @param[in] type          See @ref smtp_address_type.
 * @param[in] email         Email address.
 * @param[in] name          Name corresponding to @p email.
 * @param[in] expect_status Expected return code from @ref smtp_address_add.
 */
static void
smtp_address_add_check(const enum smtp_address_type type,
                       const char *const email,
                       const char *const name,
                       const enum smtp_status_code expect_status){
  g_rc = smtp_address_add(g_config.smtp, type, email, name);
  assert(g_rc == expect_status);
}

/**
 * Call @ref smtp_header_add and check the result.
 *
 * @param[in] key           Key name for new header.
 * @param[in] value         Value for new header.
 * @param[in] expect_status Expected return code from @ref smtp_header_add.
 */
static void
smtp_header_add_check(const char *const key,
                      const char *const value,
                      const enum smtp_status_code expect_status){
  g_rc = smtp_header_add(g_config.smtp, key, value);
  assert(g_rc == expect_status);
}

/**
 * Call @ref smtp_mail and check the result.
 *
 * @param[in] body          Email body text.
 * @param[in] expect_status Expected return code from @ref smtp_mail.
 */
static void
smtp_mail_check(const char *const body,
                const enum smtp_status_code expect_status){
  g_rc = smtp_mail(g_config.smtp, body);
  assert(g_rc == expect_status);
}

/**
 * Call @ref smtp_close and check the result.
 *
 * @param[in] expect_status Expected return code from @ref smtp_close.
 */
static void
smtp_close_check(const enum smtp_status_code expect_status){
  g_rc = smtp_close(g_config.smtp);
  assert(g_rc == expect_status);
}

/**
 * Opens an smtp connection using @ref smtp_open and adds default addresses.
 *
 * Uses default connection and flag parameters and ensures the return status
 * gets set to SMTP_STATUS_OK. Adds the FROM, TO, and CC email addresses
 * given in the config file. This function should always succeed.
 */
static void
test_smtp_open_default(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_CC,
                         g_config.email_to_2,
                         SMTP_TEST_DEFAULT_CC_NAME,
                         SMTP_STATUS_OK);

  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "test.txt",
                                 "test attachment",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_OK);
}

/**
 * Test the @ref smtp_status_code_get function.
 */
static void
smtp_func_test_all_status_code_get(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  g_rc = smtp_status_code_get(g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_status_code_set(g_config.smtp, SMTP_STATUS_NOMEM);
  g_rc = smtp_status_code_get(g_config.smtp);
  assert(g_rc == SMTP_STATUS_NOMEM);
  smtp_status_code_clear(g_config.smtp);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send a test email with the given parameters.
 *
 * See @ref smtp_open and @ref smtp_auth for more information about this
 * functions parameters.
 *
 * @param[in] port                Server connection port.
 * @param[in] connection_security Connection security settings.
 * @param[in] flags               Miscellaneous configuration flags.
 * @param[in] auth_method         Authentication method.
 * @param[in] cafile              Path to certificate file.
 * @param[in] subject             Email subject line.
 * @param[in] body                Email body.
 */
static void
smtp_func_test_send_email(const char *const port,
                          enum smtp_connection_security connection_security,
                          enum smtp_flag flags,
                          enum smtp_authentication_method auth_method,
                          const char *const cafile,
                          const char *const subject,
                          const char *const body){
  g_rc = smtp_open(g_config.server,
                   port,
                   connection_security,
                   flags,
                   cafile,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_auth_check(auth_method, g_config.user, g_config.pass, SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_header_add_check("Subject", subject, SMTP_STATUS_OK);

  smtp_mail_check(body, SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send a test email with a specific connection security method.
 *
 * @param[in] server_port          Server port number to connect to.
 * @param[in] con_security         See @ref smtp_connection_security.
 * @param[in] security_description Description of @p connection_security.
 */
static void
smtp_func_test_connection_security(const char *const server_port,
                                   enum smtp_connection_security con_security,
                                   const char *const security_description){
  sprintf(g_config.subject,
          "SMTP Test: Connection Security %s",
          security_description);
  sprintf(g_config.body,
          "Email sent with connection security: %s",
          security_description);

  smtp_func_test_send_email(server_port,
                            con_security,
                            SMTP_TEST_DEFAULT_FLAGS,
                            SMTP_TEST_DEFAULT_AUTH_METHOD,
                            SMTP_TEST_DEFAULT_CAFILE,
                            g_config.subject,
                            g_config.body);
}

/**
 * Run through all types of SMTP connections provided in the
 * @ref smtp_connection_security.
 */
static void
smtp_func_test_all_connection_security(void){
  smtp_func_test_connection_security(g_config.port,
                                     SMTP_SECURITY_NONE,
                                     "None");
  smtp_func_test_connection_security(g_config.port,
                                     SMTP_SECURITY_STARTTLS,
                                     "STARTTLS");
  smtp_func_test_connection_security(g_config.port_tls,
                                     SMTP_SECURITY_TLS,
                                     "TLS");
}

/**
 * Send a test email with a self-signed certificate file specified in the
 * cafile parameter.
 */
static void
smtp_func_test_all_cafile(void){
  smtp_func_test_send_email(g_config.port,
                            SMTP_SECURITY_STARTTLS,
                            SMTP_DEBUG,
                            SMTP_TEST_DEFAULT_AUTH_METHOD,
                            g_config.cafile,
                            "SMTP Test: cafile",
                            g_config.cafile);
}

/**
 * Send a test email with a specific authentication method.
 *
 * @param[in] auth_method      See @ref smtp_authentication_method.
 * @param[in] auth_description Description of @p auth_method.
 */
static void
smtp_func_test_auth(enum smtp_authentication_method auth_method,
                    const char *const auth_description){
  sprintf(g_config.subject,
          "SMTP Test: AUTH %s",
          auth_description);
  sprintf(g_config.body,
          "Email authenticated using %s",
          auth_description);

  smtp_func_test_send_email(g_config.port,
                            SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                            SMTP_TEST_DEFAULT_FLAGS,
                            auth_method,
                            SMTP_TEST_DEFAULT_CAFILE,
                            g_config.subject,
                            g_config.body);
}

/**
 * Run through all types of SMTP authentication methods provided in the
 * @ref smtp_authentication_method.
 */
static void
smtp_func_test_all_auth_methods(void){
  smtp_func_test_auth(SMTP_AUTH_NONE, "NONE");
  smtp_func_test_auth(SMTP_AUTH_PLAIN, "PLAIN");
  smtp_func_test_auth(SMTP_AUTH_LOGIN, "LOGIN");
  smtp_func_test_auth(SMTP_AUTH_CRAM_MD5, "CRAM-MD5");
}

/**
 * Test harness for @ref smtp_attachment_add_path.
 *
 * @param[in] name        Name of the file to display to the recipient.
 * @param[in] path        Local file path to use as the attachment.
 * @param[in] expect_rc   Expected return code for the attachment function
 *                        and every function after that.
 */
static void
smtp_func_test_attachment_path(const char *const name,
                               const char *const path,
                               enum smtp_status_code expect_rc){
  strcpy(g_config.subject, "SMTP Test: Attachment (file path)");
  strcpy(g_config.body, "This email should contain a pdf attachment");

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_auth_check(SMTP_TEST_DEFAULT_AUTH_METHOD,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  g_rc = smtp_attachment_add_path(g_config.smtp, name, path);
  assert(g_rc == expect_rc);

  smtp_header_add_check("Subject", g_config.subject, expect_rc);

  smtp_mail_check(g_config.body, expect_rc);

  smtp_close_check(expect_rc);
}

/**
 * Test harness for @ref smtp_attachment_add_fp.
 *
 * @param[in] name        Name of the file to display to the recipient.
 * @param[in] path        Local file path to use as the attachment.
 * @param[in] expect_rc   Expected return code for the attachment function
 *                        and every function after that.
 */
static void
smtp_func_test_attachment_fp(const char *const name,
                             const char *const path,
                             enum smtp_status_code expect_rc){
  FILE *fp;
  int fp_rc;

  strcpy(g_config.subject, "SMTP Test: Attachment (fp)");
  strcpy(g_config.body, "This email should contain a pdf attachment");

  fp = fopen(path, "r");
  assert(fp != NULL);

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  g_rc = smtp_attachment_add_fp(g_config.smtp, name, fp);
  assert(g_rc == expect_rc);

  smtp_header_add_check("Subject", g_config.subject, expect_rc);

  smtp_mail_check(g_config.body, expect_rc);

  smtp_close_check(expect_rc);

  fp_rc = fclose(fp);
  assert(fp_rc == 0);
}

/**
 * Test harness for @ref smtp_attachment_add_mem.
 *
 * @param[in] num_attachment Number of attachments to send.
 */
static void
smtp_func_test_attachment_mem(size_t num_attachment){
  size_t i;
  char attachment_name[SMTP_MAX_ATTACHMENT_NAME_LEN];
  char attachment_data[100];

  sprintf(g_config.subject,
          "SMTP Test: Attachments (%u)",
          (unsigned)num_attachment);
  sprintf(g_config.body,
          "You should have %u attachments in this email. "
          "Each attachment should contain the text "
          "\"Attachment# <number>\"",
          (unsigned)num_attachment);

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_auth_check(SMTP_TEST_DEFAULT_AUTH_METHOD,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  for(i = 0; i < num_attachment; i++){
    sprintf(attachment_name, "%u.txt", (unsigned)(i + 1));
    sprintf(attachment_data, "Attachment# %u", (unsigned)(i + 1));
    g_rc = smtp_attachment_add_mem(g_config.smtp,
                                   attachment_name,
                                   attachment_data,
                                   SIZE_MAX);
    assert(g_rc == SMTP_STATUS_OK);
  }

  smtp_header_add_check("Subject", g_config.subject, SMTP_STATUS_OK);

  smtp_mail_check(g_config.body, SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send multiple PDF attachments.
 *
 * @param[in] num_attachments Number of attachments to send.
 */
static void
smtp_func_test_attachment_pdf(const size_t num_attachments){
  char attachment_name[SMTP_MAX_ATTACHMENT_NAME_LEN];
  size_t i;

  sprintf(g_config.subject,
          "SMTP Test: PDF Attachments (%u)",
          (unsigned)num_attachments);
  sprintf(g_config.body,
          "You should have %u PDF attachments in this email. "
          "Each attachment should contain the text "
          "\"SMTP TEST PDF ATTACHMENT\"",
          (unsigned)num_attachments);
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);
  smtp_auth_check(SMTP_TEST_DEFAULT_AUTH_METHOD,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  for(i = 0; i < num_attachments; i++){
    sprintf(attachment_name, "test-%u.pdf", (unsigned)(i + 1));
    g_rc = smtp_attachment_add_path(g_config.smtp,
                                    attachment_name,
                                    "test/test.pdf");
    assert(g_rc == SMTP_STATUS_OK);
  }
  smtp_header_add_check("Subject", g_config.subject, SMTP_STATUS_OK);
  smtp_mail_check(g_config.body, SMTP_STATUS_OK);
  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test sending long text attachments.
 */
static void
smtp_func_test_attachment_long_text(void){
  char *long_text;

  /* Send Large attachment attachment with repeated text. */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_auth_check(SMTP_TEST_DEFAULT_AUTH_METHOD,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  long_text = smtp_str_repeat(STR_ALPHABET_LOWERCASE " ", 5000);
  assert(long_text);
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "alphabet-repeat.txt",
                                 long_text,
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_OK);
  free(long_text);

  smtp_header_add_check("Subject",
                        "SMTP Test: Long Text Attachment",
                        SMTP_STATUS_OK);

  smtp_mail_check("This email should contain long text attachment",
                  SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Run all tests for @ref smtp_attachment_add_mem.
 */
static void
smtp_func_test_all_attachments_mem(void){
  /* Send one attachment using the mem interface. */
  smtp_func_test_attachment_mem(1);

  /* Send 10 attachments in one email. */
  smtp_func_test_attachment_mem(10);

  /* Send 10 PDF attachments in one email. */
  smtp_func_test_attachment_pdf(10);

  smtp_func_test_attachment_long_text();
}

/**
 * Run all tests for @ref smtp_attachment_add_path.
 */
static void
smtp_func_test_all_attachments_path(void){
  /* Send a PDF test file using the path interface. */
  smtp_func_test_attachment_path("test.pdf",
                                 "test/test.pdf",
                                 SMTP_STATUS_OK);

  /* Try to send a file that does not exist. */
  smtp_func_test_attachment_path("noexist.txt",
                                 "test/noexist.txt",
                                 SMTP_STATUS_FILE);
}

/**
 * Run all tests for @ref smtp_attachment_add_fp.
 */
static void
smtp_func_test_all_attachments_fp(void){
  smtp_func_test_attachment_fp("test.pdf",
                               "test/test.pdf",
                               SMTP_STATUS_OK);
}

/**
 * Test different ways of loading file attachments onto an SMTP context.
 */
static void
smtp_func_test_all_attachments(void){
  smtp_func_test_all_attachments_path();
  smtp_func_test_all_attachments_fp();
  smtp_func_test_all_attachments_mem();
}

/**
 * Test multiple ways of sending to different recipients.
 */
static void
smtp_func_test_all_address(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  /* Multiple TO addresses. */
  smtp_header_clear_all(g_config.smtp);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: Multiple TO Addresses",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to_2,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should contain two TO recipients.",
                  SMTP_STATUS_OK);

  /* One BCC address. */
  smtp_header_clear_all(g_config.smtp);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: BCC Address",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_BCC,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_BCC_NAME,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should contain one BCC recipient.",
                  SMTP_STATUS_OK);

  /* One TO and one BCC address. */
  smtp_header_clear_all(g_config.smtp);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: TO and BCC Addresses",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_BCC,
                         g_config.email_to_2,
                         SMTP_TEST_DEFAULT_BCC_NAME,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should contain one TO and one BCC recipient.",
                  SMTP_STATUS_OK);

  /* One TO, CC, and BCC addresses. */
  smtp_header_clear_all(g_config.smtp);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: TO, CC, and BCC Addresses",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_CC,
                         g_config.email_to_2,
                         SMTP_TEST_DEFAULT_CC_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_BCC,
                         g_config.email_to_3,
                         SMTP_TEST_DEFAULT_BCC_NAME,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should contain one TO, CC, and BCC recipient.",
                  SMTP_STATUS_OK);

  /* No FROM address. */
  smtp_header_clear_all(g_config.smtp);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: No FROM address",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should not have a FROM address in the header.",
                  SMTP_STATUS_PARAM);
  smtp_status_code_clear(g_config.smtp);

  /* FROM address contains UTF-8 characters. */
  smtp_header_clear_all(g_config.smtp);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: From contains UTF-8 characters",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                          "smtp-cli€nt-t€st@somnisoft.com",
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);
  /**
   * @todo email successful queued but not sent.
   */
  smtp_mail_check("This email should contain a FROM address with UTF-8.",
                  SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test different methods of adding names to emails when calling
 * @ref smtp_address_add.
 */
static void
smtp_func_test_all_names(void){
  char *long_name;

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  /* NULL From and Blank To Names */
  smtp_address_clear_all(g_config.smtp);
  smtp_header_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: Null From Name and Blank To Name",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         NULL,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         "",
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should not have a name in From or To.",
                  SMTP_STATUS_OK);

  /* Two To Names */
  smtp_address_clear_all(g_config.smtp);
  smtp_header_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: Two To Names",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         "Email Name 1",
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to_2,
                         "Email Name 2",
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should have two addresses with different names.",
                  SMTP_STATUS_OK);

  /* Three To Names */
  smtp_address_clear_all(g_config.smtp);
  smtp_header_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: Three To Names",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         "Email Name 1",
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to_2,
                         "Email Name 2",
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to_3,
                         "Email Name 3",
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should have three addresses with different names.",
                  SMTP_STATUS_OK);

  /* Long email name */
  long_name = smtp_str_repeat(STR_ALPHABET_LOWERCASE " ", 2);
  assert(long_name);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: Long Email Names",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         long_name,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         long_name,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should have a long alphabet name.",
                  SMTP_STATUS_OK);
  free(long_name);

  /* Very long email name */
  long_name = smtp_str_repeat(STR_ALPHABET_LOWERCASE " ", 100);
  assert(long_name);
  smtp_address_clear_all(g_config.smtp);
  smtp_header_clear_all(g_config.smtp);
  smtp_header_add_check("Subject",
                        "SMTP Test: Very Long Email Names",
                        SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         long_name,
                         SMTP_STATUS_OK);
  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         long_name,
                         SMTP_STATUS_OK);
  smtp_mail_check("This email should have a very long alphabet name "
                  "repeated 100 times.",
                  SMTP_STATUS_OK);
  free(long_name);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test scenario where the caller provides a custom date value in the header.
 *
 * This should override the default date implementation which uses the current
 * local date.
 */
static void
smtp_func_test_header_custom_date(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_clear_all(g_config.smtp);

  smtp_header_add_check("Subject",
                        "SMTP Test: Custom Date",
                        SMTP_STATUS_OK);

  smtp_header_add_check("Date",
                        "Thu, 21 May 1998 05:33:29 -0700",
                        SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_mail_check("This email should contain a custom date header.",
                  SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test scenario where the caller provides a NULL value for a header.
 *
 * This should prevent that header from generating in the email.
 */
static void
smtp_func_test_header_null_no_date(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_clear_all(g_config.smtp);

  smtp_header_add_check("Subject",
                        "SMTP Test: Null Header (No Date)",
                        SMTP_STATUS_OK);

  smtp_header_add_check("Date", NULL, SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_mail_check("This email should not contain a Date header.",
                  SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test sending long headers.
 */
static void
smtp_func_test_header_long(void){
  char *long_text;

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_clear_all(g_config.smtp);

  long_text = smtp_str_repeat(STR_ALPHABET_LOWERCASE " ", 1000);
  assert(long_text);

  smtp_header_add_check("Subject", long_text, SMTP_STATUS_OK);
  smtp_header_add_check("Custom", long_text, SMTP_STATUS_OK);

  free(long_text);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_mail_check("This email should contain very long"
                  " Subject and Custom headers.",
                  SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test multiple ways of sending to different headers.
 */
static void
smtp_func_test_all_headers(void){
  smtp_func_test_header_custom_date();
  smtp_func_test_header_null_no_date();
  smtp_func_test_header_long();
}

/**
 * Test different scenarios with email bodies.
 */
static void
smtp_func_test_all_body(void){
  char *long_body;

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_add_check("Subject",
                        "SMTP Test: Very Long Email Body",
                        SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  long_body = smtp_str_repeat(STR_ALPHABET_LOWERCASE " ", 5000);
  assert(long_body);

  smtp_mail_check(long_body, SMTP_STATUS_OK);

  free(long_body);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Manipulate the number of bytes sent over the network at a time.
 */
static void
smtp_func_test_all_write(void){
  g_smtp_test_send_one_byte = 1;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_NO_CERT_VERIFY,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);
  g_smtp_test_send_one_byte = 0;
  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send a test email with debug mode disabled.
 */
static void
smtp_func_test_all_nodebug(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_NO_CERT_VERIFY,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_clear_all(g_config.smtp);

  smtp_header_add_check("Subject",
                        "SMTP Test: No Debug Mode",
                        SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_mail_check("This email sent with debug mode disabled.",
                  SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send an HTML email.
 */
static void
smtp_func_test_html(void){
  const char *const html_body =
  "<html>\n"
  " <head><title>HTML Email Example</title></head>\n"
  " <body>\n"
  "  <table style='border: 1px solid black; background-color: #a0a0a0'>\n"
  "   <caption>smtp_connection_security</caption>\n"
  "   <tr>\n"
  "    <th>Code</th>\n"
  "    <th>Description</th>\n"
  "   </tr>\n"
  "   <tr>\n"
  "    <td>SMTP_SECURITY_STARTTLS</td>\n"
  "    <td>Use STARTTLS</td>\n"
  "   </tr>\n"
  "   <tr>\n"
  "    <td>SMTP_SECURITY_TLS</td>\n"
  "    <td>Direct TLS connection</td>\n"
  "   </tr>\n"
  "   <tr>\n"
  "    <td>SMTP_SECURITY_NONE</td>\n"
  "    <td>No encryption</td>\n"
  "   </tr>\n"
  "  </table>\n"
  " </body>\n"
  "</html>";

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_clear_all(g_config.smtp);

  smtp_header_add_check("Subject",
                        "SMTP Test: HTML Email (Content-Type: text/html)",
                        SMTP_STATUS_OK);

  smtp_header_add_check("Content-Type", "text/html", SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_mail_check(html_body, SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send an HTML email with a plaintext fallback.
 */
static void
smtp_func_test_html_with_plaintext(void){
  const char *const html_body =
  "This is a multi-part message in MIME format.\r\n"
  "...\n"
  "..\n"
  ".\n"
  ".\r"
  ".\n"
  "\r\n"
  "--FEDCBA\r\n"
  "This is a multi-part message in MIME format.\r\n"
  "--ABCDEF\r\n"
  "Content-Type: text/plain; charset=\"utf-8\"\r\n"
  "Content-Transfer-Encoding: 8bit\r\n"
  "\r\n"
  "Plaintext section.\r\n"
  "\r\n"
  "--ABCDEF\r\n"
  "Content-Type: text/html; charset=\"utf-8\"\r\n"
  "Content-Transfer-Encoding: 8bit\r\n"
  "\r\n"
  "<html>\n"
  " <head><title>HTML/Plaintext Email</title></head>\n"
  " <body>\n"
  "  <h1>HTML section</h1>\n"
  "  <h2>Heading 2</h2>\n"
  " </body>\n"
  "</html>\n"
  "...\n"
  "..\n"
  ".\n"
  ".";

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_clear_all(g_config.smtp);

  smtp_header_add_check("Subject",
                        "SMTP Test: HTML Email (with plaintext)",
                        SMTP_STATUS_OK);

  smtp_header_add_check("Content-Type",
                        "multipart/alternative; boundary=\"ABCDEF\"",
                        SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  smtp_mail_check(html_body, SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Send HTML emails, overriding the Content-Type.
 */
static void
smtp_func_test_all_html(void){
  smtp_func_test_html();
  smtp_func_test_html_with_plaintext();
}

/**
 * Test failure or error conditions not covered by any of the other failure
 * tests.
 */
static void
test_failure_misc(void){
  /* Send buffer to large in @ref smtp_write. */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);
  g_rc = smtp_write(g_config.smtp, "", (size_t)INT_MAX + 1);
  assert(g_rc == SMTP_STATUS_SEND);
  smtp_close_check(SMTP_STATUS_SEND);

  /* Memory allocation failure in smtp_puts_debug - the error gets ignored. */
  g_smtp_test_err_malloc_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_malloc_ctr = -1;
  assert(g_rc == SMTP_STATUS_OK);
  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test failure points in the @ref smtp_open function.
 */
static void
test_failure_open(void){
  /* Initial memory allocation failure for the SMTP client context. */
  g_smtp_test_err_calloc_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_calloc_ctr = -1;
  assert(g_rc == SMTP_STATUS_NOMEM);
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Invalid hostname should cause getaddrinfo() to fail. */
  g_rc = smtp_open(NULL,
                   NULL,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_CONNECT);
  smtp_close_check(SMTP_STATUS_CONNECT);

  /* socket() function failure. */
  g_smtp_test_err_socket_ctr = 0;
  g_rc = smtp_open(g_config.server,
                 g_config.port,
                 SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                 SMTP_TEST_DEFAULT_FLAGS,
                 SMTP_TEST_DEFAULT_CAFILE,
                 &g_config.smtp);
  g_smtp_test_err_socket_ctr = -1;
  assert(g_rc == SMTP_STATUS_CONNECT);
  smtp_close_check(SMTP_STATUS_CONNECT);

  /* connect() function failure. */
  g_smtp_test_err_connect_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_connect_ctr = -1;
  assert(g_rc == SMTP_STATUS_CONNECT);
  smtp_close_check(SMTP_STATUS_CONNECT);

  /* SSL_CTX_new() failure. */
  g_smtp_test_err_ssl_ctx_new_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_ctx_new_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* ERR_peek_error() failure. */
  g_smtp_test_err_err_peek_error_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_err_peek_error_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* SSL_new() failure. */
  g_smtp_test_err_ssl_new_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_new_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* BIO_new_socket() failure. */
  g_smtp_test_err_bio_new_socket_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_bio_new_socket_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* SSL_connect() failure. */
  g_smtp_test_err_ssl_connect_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_connect_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* SSL_do_handshake() failure. */
  g_smtp_test_err_ssl_do_handshake_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_do_handshake_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /*
   * Ensure self-signed certificate throws an error. This error will occur by
   * default since the test server uses a self-signed certificate.
   */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_DEBUG,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* SSL_CTX_load_verify_locations() failure. */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_DEBUG,
                   "test/config/file_does_not_exist",
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* SSL_get_peer_certificate() failure. */
  g_smtp_test_err_ssl_get_peer_certificate_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_DEBUG,
                   g_config.cafile,
                   &g_config.smtp);
  g_smtp_test_err_ssl_get_peer_certificate_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* X509_check_host() failure.  */
  g_smtp_test_err_x509_check_host_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_DEBUG,
                   g_config.cafile,
                   &g_config.smtp);
  g_smtp_test_err_x509_check_host_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /*
   * TLS failure in @ref smtp_initiate_handshake (1) when using direct
   * TLS connection.
   */
  g_smtp_test_err_ssl_ctx_new_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_TLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_ctx_new_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* @ref smtp_initiate_handshake failure in (2). */
  g_smtp_test_err_recv_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_recv_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* @ref smtp_initiate_handshake failure in (3). */
  g_smtp_test_err_send_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_send_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* @ref smtp_initiate_handshake STARTTLS send failure in (4). */
  g_smtp_test_err_send_ctr = 1;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_send_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* @ref smtp_initiate_handshake failed response to STARTTLS in (4). */
  g_smtp_test_err_recv_ctr = 2;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_recv_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* @ref smtp_initiate_handshake second EHLO in (4). */
  g_smtp_test_err_ssl_write_ctr = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_write_ctr = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* Failure in @ref BIO_should_retry. */
  g_smtp_test_err_ssl_read_ctr = 0;
  g_smtp_test_err_bio_should_retry_ctr = 0;
  g_smtp_test_err_bio_should_retry_rc = -1;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_read_ctr = -1;
  g_smtp_test_err_bio_should_retry_ctr = -1;
  g_smtp_test_err_bio_should_retry_rc = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);

  /* Failure in @ref SSL_Read but re-reading caused by @ref BIO_should_retry. */
  g_smtp_test_err_ssl_read_ctr = 0;
  g_smtp_test_err_bio_should_retry_ctr = -1;
  g_smtp_test_err_bio_should_retry_rc = 1;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_ssl_read_ctr = -1;
  g_smtp_test_err_bio_should_retry_ctr = -1;
  g_smtp_test_err_bio_should_retry_rc = -1;
  assert(g_rc == SMTP_STATUS_OK);
  smtp_close_check(SMTP_STATUS_OK);

  /* Test server prematurely ending the connection with no bytes to read. */
  g_smtp_test_err_recv_ctr = 2;
  g_smtp_test_err_recv_rc = 0;
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  g_smtp_test_err_recv_ctr = -1;
  g_smtp_test_err_recv_rc = -1;
  assert(g_rc == SMTP_STATUS_HANDSHAKE);
  smtp_close_check(SMTP_STATUS_HANDSHAKE);
}

/**
 * Test different error results in the address functions, including memory
 * allocation failures.
 */
static void
test_failure_address_add(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  /* Invalid SMTP status code. */
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_NOMEM);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_NOMEM);

  /* Invalid email address. */
  smtp_status_code_clear(g_config.smtp);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         "<invalid>",
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_PARAM);

  /* Invalid email name. */
  smtp_status_code_clear(g_config.smtp);
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                          "\"invalid\"",
                         SMTP_STATUS_PARAM);

  /* Wrap when trying to increase size of address list. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_si_add_size_t_ctr = 0;
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;

  /* Memory allocation failed while trying to increase size of address list. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_realloc_ctr = 0;
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;

  /* Failed to duplicate email string. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_malloc_ctr = 0;
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;

  /* Failed to duplicate name string. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_malloc_ctr = 1;
  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;

  smtp_close_check(SMTP_STATUS_NOMEM);
}

/**
 * Test different error results in the attachment functions, including memory
 * allocation failures.
 */
static void
test_failure_attachment_add(void){
  FILE *fp;
  int fp_rc;

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);


  /* Invalid SMTP status code. */
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_PARAM);
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "valid",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_PARAM);


  /* Invalid filename parameter. */
  smtp_status_code_clear(g_config.smtp);
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "\"invalid\"",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_PARAM);

  /* Wrap when increasing the attachment list size. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_si_add_size_t_ctr = 0;
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "valid",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;

  /* Memory allocation failure while increasing the attachment list size. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_realloc_ctr = 0;
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "valid",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;

  /* Memory allocation failure while using smtp_strdup on file name. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_malloc_ctr = 0;
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "valid",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;


  /* Memory allocation failure while using smtp_base64_encode. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_calloc_ctr = 0;
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "valid",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_NOMEM);
  g_smtp_test_err_calloc_ctr = -1;

  /* Memory allocation failure when splitting base64 lines into chunks. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_calloc_ctr = 1;
  g_rc = smtp_attachment_add_mem(g_config.smtp,
                                 "valid",
                                 "test",
                                 SIZE_MAX);
  assert(g_rc == SMTP_STATUS_NOMEM);
  g_smtp_test_err_calloc_ctr = -1;

  /* Invalid SMTP status code. */
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_PARAM);
  g_rc = smtp_attachment_add_fp(g_config.smtp, "test", stdin);
  assert(g_rc == SMTP_STATUS_PARAM);

  /* @ref smtp_ffile_get_contents memory allocation failure. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_realloc_ctr = 0;
  g_rc = smtp_attachment_add_fp(g_config.smtp, "test", stdin);
  g_smtp_test_err_realloc_ctr = -1;
  assert(g_rc == SMTP_STATUS_NOMEM);

  /* @ref smtp_ffile_get_contents fread error. */
  smtp_status_code_clear(g_config.smtp);
  fp = fopen("COPYING", "r");
  assert(fp);
  g_smtp_test_err_ferror_ctr = 0;
  g_rc = smtp_attachment_add_fp(g_config.smtp, "test", fp);
  g_smtp_test_err_ferror_ctr = -1;
  assert(g_rc == SMTP_STATUS_FILE);
  fp_rc = fclose(fp);
  assert(fp_rc == 0);

  /* @ref smtp_file_get_contents memory allocation failure. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_realloc_ctr = 0;
  g_rc = smtp_attachment_add_path(g_config.smtp, "test", "COPYING");
  g_smtp_test_err_realloc_ctr = -1;
  assert(g_rc == SMTP_STATUS_NOMEM);

  /* Invalid SMTP status code. */
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_PARAM);
  g_rc = smtp_attachment_add_path(g_config.smtp, "test", "test.txt");
  assert(g_rc == SMTP_STATUS_PARAM);

  smtp_close_check(SMTP_STATUS_PARAM);
}

/**
 * Test different error results in the @ref smtp_header_add function,
 * including memory allocation failures.
 */
static void
test_failure_header_add(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  /* Invalid SMTP status code. */
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_NOMEM);
  smtp_header_add_check("key", "value", SMTP_STATUS_NOMEM);

  /* Invalid header key. */
  smtp_status_code_clear(g_config.smtp);
  smtp_header_add_check("invalid:", "value", SMTP_STATUS_PARAM);

  /* Invalid header value. */
  smtp_status_code_clear(g_config.smtp);
  smtp_header_add_check("key", "invalid\n", SMTP_STATUS_PARAM);

  /* Wrap when increasing the header list size. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_si_add_size_t_ctr = 0;
  smtp_header_add_check("key", "value", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;

  /* Memory allocation failure while trying to increase header list size. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_realloc_ctr = 0;
  smtp_header_add_check("key", "value", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;

  /* Failed to strdup header key. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_malloc_ctr = 0;
  smtp_header_add_check("key", "value", SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;

  /* Failed to strdup header value. */
  smtp_status_code_clear(g_config.smtp);
  g_smtp_test_err_malloc_ctr = 1;
  smtp_header_add_check("key", "value", SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;

  smtp_close_check(SMTP_STATUS_NOMEM);
}

/**
 * Test different error results @ref smtp_status_code_set function.
 */
static void
test_failure_status_code_set(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  g_rc = smtp_status_code_set(g_config.smtp, (enum smtp_status_code)-1);
  assert(g_rc == SMTP_STATUS_PARAM);

  g_rc = smtp_status_code_set(g_config.smtp, SMTP_STATUS__LAST);
  assert(g_rc == SMTP_STATUS_PARAM);

  g_rc = smtp_status_code_clear(g_config.smtp);
  assert(g_rc == SMTP_STATUS_PARAM);

  g_rc = smtp_status_code_get(g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Test different error conditions in the @ref smtp_mail function.
 */
static void
test_failure_mail(void){
  /* Invalid SMTP client context. */
  test_smtp_open_default();
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_NOMEM);
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Wrap in @ref smtp_mail_envelope_header size calculation. */
  test_smtp_open_default();
  g_smtp_test_strlen_custom_ret = 1;
  g_smtp_test_strlen_ret_value = SIZE_MAX - 3;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_strlen_custom_ret = 0;
  g_smtp_test_strlen_ret_value = 0;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /*
   * Memory allocation failure in the first call to
   * @ref smtp_mail_envelope_header.
   */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 0;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Send failure in @ref smtp_mail_envelope_header. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 0;
  g_smtp_test_err_ssl_write_ctr = 0;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* Read failure in @ref smtp_mail_envelope_header. */
  test_smtp_open_default();
  g_smtp_test_err_recv_ctr = 0;
  g_smtp_test_err_ssl_read_ctr = 0;
  smtp_mail_check("body", SMTP_STATUS_RECV);
  g_smtp_test_err_recv_ctr = -1;
  g_smtp_test_err_ssl_read_ctr = -1;
  smtp_close_check(SMTP_STATUS_RECV);

  /* Send failure in the second call to @ref smtp_mail_envelope_header. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 1;
  g_smtp_test_err_ssl_write_ctr = 1;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* Failed to send DATA command. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 3;
  g_smtp_test_err_ssl_write_ctr = 3;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* Failed to read response to DATA command. */
  test_smtp_open_default();
  g_smtp_test_err_recv_ctr = 3;
  g_smtp_test_err_ssl_read_ctr = 3;
  smtp_mail_check("body", SMTP_STATUS_SERVER_RESPONSE);
  g_smtp_test_err_recv_ctr = -1;
  g_smtp_test_err_ssl_read_ctr = -1;
  smtp_close_check(SMTP_STATUS_SERVER_RESPONSE);

  /* Failed to generate date string in @ref smtp_date_rfc_2822. */
  test_smtp_open_default();
  g_smtp_test_err_localtime_r_ctr = 0;
  smtp_mail_check("body", SMTP_STATUS_DATE);
  g_smtp_test_err_localtime_r_ctr = -1;
  smtp_close_check(SMTP_STATUS_DATE);

  /* Failed to add Date header to list using @ref smtp_header_add. */
  test_smtp_open_default();
  g_smtp_test_err_realloc_ctr = 0;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* 1st wrap in @ref smtp_append_address_to_header */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 30;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* 2nd wrap in @ref smtp_append_address_to_header */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 31;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* 3rd wrap in @ref smtp_append_address_to_header */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 32;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /*
   * Failed to add FROM address to header using
   * @ref smtp_append_address_to_header.
   */
  test_smtp_open_default();
  g_smtp_test_err_realloc_ctr = 1;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /*
   * Failed to add TO address to header using
   * @ref smtp_append_address_to_header.
   */
  test_smtp_open_default();
  g_smtp_test_err_realloc_ctr = 3;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /*
   * Failed to add CC address to header using
   * @ref smtp_append_address_to_header.
   */
  test_smtp_open_default();
  g_smtp_test_err_realloc_ctr = 5;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* 1st wrap in @ref smtp_print_header. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 48;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* 2nd wrap in @ref smtp_print_header. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 49;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Failed memory allocation in @ref smtp_print_header. */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 19;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Failed @ref smtp_fold_whitespace in @ref smtp_print_header. */
  test_smtp_open_default();
  g_smtp_test_err_realloc_ctr = 8;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Failed @ref smtp_puts_terminate in @ref smtp_print_header. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 4;
  g_smtp_test_err_ssl_write_ctr = 4;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /*
   * Failure in @ref smtp_print_mime_email ->
   * @ref smtp_print_mime_header_and_body   ->
   * @ref smtp_str_replace.
   */
  test_smtp_open_default();
  g_smtp_test_err_realloc_ctr = 11;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_realloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Wrap in @ref smtp_print_mime_header_and_body size calculation. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 92;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /*
   * Memory allocation failure in @ref smtp_print_mime_email ->
   * @ref smtp_print_mime_header_and_body   ->
   * malloc after @ref smtp_str_replace.
   */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 31;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /*
   * Send failure in @ref smtp_print_mime_email ->
   * @ref smtp_print_mime_header_and_body   ->
   * @ref smtp_puts.
   */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 8;
  g_smtp_test_err_ssl_write_ctr = 8;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* 1st wrap in @ref smtp_print_mime_attachment. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 94;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* 2nd wrap in @ref smtp_print_mime_attachment. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 95;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Memory allocation failure in @ref smtp_print_mime_attachment. */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 33;
  smtp_mail_check("body", SMTP_STATUS_NOMEM);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_NOMEM);

  /* Send failure in @ref smtp_print_mime_attachment. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 9;
  g_smtp_test_err_ssl_write_ctr = 9;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* Send failure in @ref smtp_print_mime_end. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 10;
  g_smtp_test_err_ssl_write_ctr = 10;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* Failed to send end of DATA segment. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr = 11;
  g_smtp_test_err_ssl_write_ctr = 11;
  smtp_mail_check("body", SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_SEND);

  /* Invalid server response on DATA termination. */
  test_smtp_open_default();
  g_smtp_test_err_recv_ctr = 4;
  g_smtp_test_err_ssl_read_ctr = 4;
  smtp_mail_check("body", SMTP_STATUS_SERVER_RESPONSE);
  g_smtp_test_err_recv_ctr = -1;
  g_smtp_test_err_ssl_read_ctr = -1;
  smtp_close_check(SMTP_STATUS_SERVER_RESPONSE);
}

/**
 * Test different error conditions in the @ref smtp_close function.
 */
static void
test_failure_close(void){
  /* Failed to send the QUIT command. */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);
  g_smtp_test_err_send_ctr = 0;
  g_smtp_test_err_ssl_write_ctr = 0;
  smtp_close_check(SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;

  /* Failed to close the socket file descriptor. */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);
  g_smtp_test_err_close_ctr = 0;
  smtp_close_check(SMTP_STATUS_CLOSE);
  g_smtp_test_err_close_ctr = -1;

  /* Failed to send QUIT and close the socket file descriptor. */
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);
  g_smtp_test_err_send_ctr = 0;
  g_smtp_test_err_ssl_write_ctr = 0;
  g_smtp_test_err_close_ctr = 0;
  smtp_close_check(SMTP_STATUS_SEND);
  g_smtp_test_err_send_ctr = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  g_smtp_test_err_close_ctr = -1;
}

/**
 * Test different error results in the auth functions, including memory
 * allocation failures and invalid credentials.
 */
static void
test_failure_auth(void){
  smtp_test_sleep(15);

  /* Invalid SMTP status code. */
  test_smtp_open_default();
  smtp_status_code_set(g_config.smtp, SMTP_STATUS_NOMEM);
  smtp_auth_check(SMTP_AUTH_NONE,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_NOMEM);
  smtp_close_check(SMTP_STATUS_NOMEM);

  smtp_test_sleep(15);

  /* Invalid authentication method. */
  test_smtp_open_default();
  smtp_auth_check((enum smtp_authentication_method)-1,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_PARAM);
  smtp_close_check(SMTP_STATUS_PARAM);

  smtp_test_sleep(15);

  /* PLAIN - Invalid credentials. */
  test_smtp_open_default();
  smtp_auth_check(SMTP_AUTH_PLAIN, "invalid", "invalid", SMTP_STATUS_AUTH);
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - Wrap in 1st calculation for memory allocation in (1). */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 0;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - Wrap in 2nd calculation for memory allocation in (1). */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 1;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - Memory allocation failure in (1). */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 0;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - @ref smtp_base64_encode failure in (2). */
  test_smtp_open_default();
  g_smtp_test_err_calloc_ctr = 0;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_calloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - Wrap in calculation for memory allocation in (3). */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 2;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - Memory allocation failure in (3). */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 1;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* PLAIN - @ref smtp_puts failure in (3). */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr      = 0;
  g_smtp_test_err_ssl_write_ctr = 0;
  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_send_ctr      = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - @ref smtp_base64_encode failure in (1). */
  test_smtp_open_default();
  g_smtp_test_err_calloc_ctr = 0;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_calloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - Wrap in calculation for memory allocation in (2). */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 0;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - Memory allocation failure in (2). */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 0;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - @ref smtp_puts send failure in (2). */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr      = 0;
  g_smtp_test_err_ssl_write_ctr = 0;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_send_ctr      = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - Response read error in (2). */
  test_smtp_open_default();
  g_smtp_test_err_recv_ctr     = 0;
  g_smtp_test_err_ssl_read_ctr = 0;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_recv_ctr     = -1;
  g_smtp_test_err_ssl_read_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - @ref smtp_base64_encode failure in (3). */
  test_smtp_open_default();
  g_smtp_test_err_calloc_ctr = 2;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_calloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - @ref smtp_puts_terminate failure in (3). */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr      = 1;
  g_smtp_test_err_ssl_write_ctr = 1;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_send_ctr      = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - @ref smtp_puts_terminate wrap in (3). */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 7;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - @ref smtp_puts_terminate memory allocation failure in (3). */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 3;
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* LOGIN - Invalid credentials in (3). */
  test_smtp_open_default();
  smtp_auth_check(SMTP_AUTH_LOGIN,
                  "invalid",
                  "invalid",
                  SMTP_STATUS_AUTH);
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (1) @ref smtp_puts failure. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr      = 0;
  g_smtp_test_err_ssl_write_ctr = 0;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_send_ctr      = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (1) Response read error. */
  test_smtp_open_default();
  g_smtp_test_err_recv_ctr     = 0;
  g_smtp_test_err_ssl_read_ctr = 0;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_recv_ctr     = -1;
  g_smtp_test_err_ssl_read_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (1) Response memory allocation error. */
  test_smtp_open_default();
  g_smtp_test_err_calloc_ctr = 0;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_calloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (1) Server response bad. */
  test_smtp_open_default();
  g_smtp_test_err_recv_ctr     = 0;
  g_smtp_test_err_ssl_read_ctr = 0;
  strcpy(g_smtp_test_err_recv_bytes, "535 authentication failed");
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_recv_bytes[0] = '\0';
  g_smtp_test_err_recv_ctr     = -1;
  g_smtp_test_err_ssl_read_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (2) @ref smtp_base64_decode failure. */
  test_smtp_open_default();
  g_smtp_test_err_calloc_ctr = 1;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_calloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (3) @ref HMAC failure. */
  test_smtp_open_default();
  g_smtp_test_err_hmac_ctr = 0;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_hmac_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (4) @ref smtp_bin2hex failure. */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 2;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (5) Wrap in 1st memory calculation. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 8;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (5) Wrap in 2nd memory calculation. */
  test_smtp_open_default();
  g_smtp_test_err_si_add_size_t_ctr = 9;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_si_add_size_t_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (5) Memory allocation failure. */
  test_smtp_open_default();
  g_smtp_test_err_malloc_ctr = 3;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_malloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (6) @ref smtp_base64_encode failure. */
  test_smtp_open_default();
  g_smtp_test_err_calloc_ctr = 2;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_calloc_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (7) @ref smtp_puts_terminate failure. */
  test_smtp_open_default();
  g_smtp_test_err_send_ctr      = 1;
  g_smtp_test_err_ssl_write_ctr = 1;
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_AUTH);
  g_smtp_test_err_send_ctr      = -1;
  g_smtp_test_err_ssl_write_ctr = -1;
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);

  /* CRAM-MD5 (7) Invalid credentials. */
  test_smtp_open_default();
  smtp_auth_check(SMTP_AUTH_CRAM_MD5,
                  "invalid",
                  "invalid",
                  SMTP_STATUS_AUTH);
  smtp_close_check(SMTP_STATUS_AUTH);

  smtp_test_sleep(15);
}

/**
 * Simulate a timeout when reading server response.
 */
static void
test_failure_timeout(void){
  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_TEST_DEFAULT_CONNECTION_SECURITY,
                   SMTP_TEST_DEFAULT_FLAGS,
                   SMTP_TEST_DEFAULT_CAFILE,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  g_smtp_test_err_select_ctr = 0;
  smtp_mail_check("body", SMTP_STATUS_RECV);
  g_smtp_test_err_select_ctr = -1;

  smtp_close_check(SMTP_STATUS_RECV);
}

/**
 * Test multiple failure modes when using the high-level interfaces.
 */
static void
test_all_failure_modes(void){
  test_failure_misc();
  test_failure_open();
  test_failure_auth();
  test_failure_address_add();
  test_failure_attachment_add();
  test_failure_header_add();
  test_failure_status_code_set();
  test_failure_mail();
  test_failure_close();
  test_failure_timeout();
}

/**
 * Run the functional tests on local postfix server.
 *
 * This configuration handles most of the functional testing and includes:
 *   - Failure modes.
 *   - Different combinations of connection and authentication methods.
 *   - Multiple attachments.
 *   - Multiple recipients
 */
static void
smtp_func_test_server_postfix(void){
  int rc;

  rc = smtp_test_config_load_from_file("test/config/postfix.txt");
  assert(rc == 0);

  test_all_failure_modes();

  smtp_test_sleep(60);

  smtp_func_test_all_status_code_get();
  smtp_func_test_all_connection_security();
  smtp_func_test_all_cafile();
  smtp_func_test_all_auth_methods();
  smtp_func_test_all_attachments();
  smtp_func_test_all_address();
  smtp_func_test_all_names();
  smtp_func_test_all_headers();
  smtp_func_test_all_body();
  smtp_func_test_all_write();
  smtp_func_test_all_nodebug();
  smtp_func_test_all_html();
}

/**
 * Open and close a connection to secureserver.net.
 */
static void
smtp_func_test_server_secureserver(void){
  const char *const server = "smtpout.secureserver.net";
  const char *port;
  enum smtp_connection_security conn_security;
  struct smtp *smtp;
  unsigned int connection_i;
  unsigned int i;

  for(connection_i = 0; connection_i < 2; connection_i++){
    if(connection_i == 0){
      port = "25";
      conn_security = SMTP_SECURITY_STARTTLS;
    }
    else{
      port = "465";
      conn_security = SMTP_SECURITY_TLS;
    }
    for(i = 0; i < 4; i++){
      fprintf(stderr, "%s: %s: %u\n", server, port, i + 1);
      g_rc = smtp_open(server,
                       port,
                       conn_security,
                       SMTP_DEBUG,
                       NULL,
                       &smtp);
      assert(g_rc == SMTP_STATUS_OK);

      g_rc = smtp_close(smtp);
      assert(g_rc == SMTP_STATUS_OK);

      smtp_test_sleep(1);
    }
  }
}

/**
 * Send attachment to test gmail account.
 */
static void
smtp_func_test_gmail_attachment(void){
  const char *const name = "test.pdf";
  const char *const path = "test/test.pdf";

  strcpy(g_config.subject, "SMTP Test: GMail Attachment (file path)");
  strcpy(g_config.body, "This email should contain a pdf attachment");

  g_rc = smtp_open(g_config.server,
                   g_config.port,
                   SMTP_SECURITY_STARTTLS,
                   SMTP_DEBUG,
                   NULL,
                   &g_config.smtp);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_auth_check(SMTP_AUTH_PLAIN,
                  g_config.user,
                  g_config.pass,
                  SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_FROM,
                         g_config.email_from,
                         SMTP_TEST_DEFAULT_FROM_NAME,
                         SMTP_STATUS_OK);

  smtp_address_add_check(SMTP_ADDRESS_TO,
                         g_config.email_to,
                         SMTP_TEST_DEFAULT_TO_NAME,
                         SMTP_STATUS_OK);

  g_rc = smtp_attachment_add_path(g_config.smtp, name, path);
  assert(g_rc == SMTP_STATUS_OK);

  smtp_header_add_check("Subject", g_config.subject, SMTP_STATUS_OK);

  smtp_mail_check(g_config.body, SMTP_STATUS_OK);

  smtp_close_check(SMTP_STATUS_OK);
}

/**
 * Run the functional tests on the test SMTP gmail account.
 *
 * This only sends one email using a test gmail account. Most of the tests
 * have been designed to work with a local postfix server instance.
 */
static void
smtp_func_test_server_gmail(void){
  int rc;

  rc = smtp_test_config_load_from_file("test/config/gmail.txt");
  assert(rc == 0);

  fprintf(stderr, "SMTP TEST: sending test email using gmail account");
  smtp_func_test_send_email(g_config.port,
                            SMTP_SECURITY_STARTTLS,
                            SMTP_DEBUG,
                            SMTP_AUTH_PLAIN,
                            SMTP_TEST_DEFAULT_CAFILE,
                            "SMTP Test: gmail",
                            "test email sent through gmail server");

  smtp_func_test_gmail_attachment();
}

/**
 * Run through all functional/integration tests for each test SMTP server.
 */
static void
smtp_func_test_all(void){
  smtp_func_test_server_secureserver();
  smtp_func_test_server_gmail();
  smtp_func_test_server_postfix();
}

/**
 * Configuration flags for the smtp testing framework.
 */
enum smtp_test_flags{
  /**
   * Only run the unit tests, skipping all functional testing.
   */
  SMTP_TEST_UNIT_TESTING_ONLY = 1 << 0
};

/**
 * Program configuration parameters.
 */
struct smtp_test{
  /**
   * See @ref smtp_test_flags.
   */
  enum smtp_test_flags flags;
};

/**
 * Main testing program entry point for testing the smtp-client library.
 *
 * This program supports the following options:
 *   - u - Only run unit tests, skipping the functional testing.
 *
 * @param[in] argc Number of arguments in @p argv.
 * @param[in] argv String array containing the program name and any optional
 *                 parameters described above.
 * @retval 0 All tests passed.
 * @retval 1 Error.
 */
int main(int argc, char *argv[]){
  struct smtp_test smtp_test;
  int c;

  memset(&smtp_test, 0, sizeof(smtp_test));

  while((c = getopt(argc, argv, "u")) != -1){
    switch(c){
      case 'u':
        smtp_test.flags |= SMTP_TEST_UNIT_TESTING_ONLY;
        break;
      default:
        return 1;
    }
  }
  argc -= optind;
  argv += optind;

  smtp_unit_test_all();

  if(!(smtp_test.flags & SMTP_TEST_UNIT_TESTING_ONLY)){
    smtp_func_test_all();
  }

  return 0;
}

