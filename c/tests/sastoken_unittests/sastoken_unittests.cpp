// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <time.h>


#include "agenttime.h"
#include "crt_abstractions.h"
#include "sastoken.h"
#include "hmacsha256.h"
#include "urlencode.h"
#include "base64.h"
#include "buffer_.h"

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"

#define TEST_STRING_HANDLE (STRING_HANDLE)0x46
#define TEST_NULL_STRING_HANDLE (STRING_HANDLE)0x00
#define TEST_BUFFER_HANDLE (BUFFER_HANDLE)0x47
#define TEST_NULL_BUFFER_HANDLE (BUFFER_HANDLE)0x00
#define TEST_SCOPE_HANDLE (STRING_HANDLE)0x48
#define TEST_KEY_HANDLE (STRING_HANDLE)0x49
#define TEST_KEYNAME_HANDLE (STRING_HANDLE)0x50
#define TEST_HASH_HANDLE (BUFFER_HANDLE)0x51
#define TEST_TOBEHASHED_HANDLE (STRING_HANDLE)0x52
#define TEST_RESULT_HANDLE (STRING_HANDLE)0x53
#define TEST_BASE64SIGNATURE_HANDLE (STRING_HANDLE)0x54
#define TEST_URLENCODEDSIGNATURE_HANDLE (STRING_HANDLE)0x55
#define TEST_DECODEDKEY_HANDLE (BUFFER_HANDLE)0x56
#define TEST_TIME_T ((time_t)3600)
#define TEST_PTR_DECODEDKEY (unsigned char*)0x123
#define TEST_LENGTH_DECODEDKEY (size_t)32
#define TEST_PTR_TOBEHASHED (const char*)0x456
#define TEST_LENGTH_TOBEHASHED (size_t)456
#define TEST_EXPIRY ((size_t)7200)

static char TEST_CHAR_ARRAY[10] = "ABCD";

static unsigned char TEST_UNSIGNED_CHAR_ARRAY[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
static char TEST_TOKEN_EXPIRATION_TIME[32] = "7200";

static MICROMOCK_MUTEX_HANDLE g_testByTest;
static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

TYPED_MOCK_CLASS(CSASTokenMocks, CGlobalMock)
{
public:
    /* String Mocks*/
    MOCK_STATIC_METHOD_0(, STRING_HANDLE, STRING_new)
    MOCK_METHOD_END(STRING_HANDLE, (STRING_HANDLE)malloc(1))


    MOCK_STATIC_METHOD_2(, int, STRING_concat, STRING_HANDLE, handle, const char*, s2)
    MOCK_METHOD_END(int, 0)

    MOCK_STATIC_METHOD_2(, int, STRING_concat_with_STRING, STRING_HANDLE, s1, STRING_HANDLE, s2)
    MOCK_METHOD_END(int, 0)

    MOCK_STATIC_METHOD_1(, void, STRING_delete, STRING_HANDLE, handle)
    MOCK_VOID_METHOD_END()

    MOCK_STATIC_METHOD_1(, const char*, STRING_c_str, STRING_HANDLE, s)
    MOCK_METHOD_END(const char*, &TEST_CHAR_ARRAY[0])

    MOCK_STATIC_METHOD_1(, size_t, STRING_length, STRING_HANDLE, handle);
    MOCK_METHOD_END(size_t, 1)

    MOCK_STATIC_METHOD_2(, int, STRING_copy, STRING_HANDLE, handle, const char*, s2)
    MOCK_METHOD_END(int, 0)


    /* BUFFER Mocks */
    MOCK_STATIC_METHOD_0(, BUFFER_HANDLE, BUFFER_new)
    MOCK_METHOD_END(BUFFER_HANDLE, (BUFFER_HANDLE)malloc(1))

    MOCK_STATIC_METHOD_1(, void, BUFFER_delete, BUFFER_HANDLE, s)
    MOCK_VOID_METHOD_END()

    MOCK_STATIC_METHOD_1(, unsigned char*, BUFFER_u_char, BUFFER_HANDLE, handle);
    MOCK_METHOD_END(unsigned char*, &TEST_UNSIGNED_CHAR_ARRAY[0])

    MOCK_STATIC_METHOD_1(, size_t, BUFFER_length, BUFFER_HANDLE, handle);
    MOCK_METHOD_END(size_t, 1)

    /*Sundry mocks*/
    MOCK_STATIC_METHOD_1(, STRING_HANDLE, Base64_Encode, BUFFER_HANDLE, input)
    MOCK_METHOD_END(STRING_HANDLE, (STRING_HANDLE)malloc(1))

    MOCK_STATIC_METHOD_1(, BUFFER_HANDLE, Base64_Decoder, const char*, input)
    MOCK_METHOD_END(BUFFER_HANDLE, (BUFFER_HANDLE)malloc(1))

    MOCK_STATIC_METHOD_1(, STRING_HANDLE, URL_Encode, STRING_HANDLE, input)
    MOCK_METHOD_END(STRING_HANDLE, (STRING_HANDLE)malloc(1))

    MOCK_STATIC_METHOD_5(, HMACSHA256_RESULT, HMACSHA256_ComputeHash, const unsigned char*, key, size_t, keyLen, const unsigned char*, payload, size_t, payloadLen, BUFFER_HANDLE, hash)
    MOCK_METHOD_END(HMACSHA256_RESULT, HMACSHA256_OK)

    MOCK_STATIC_METHOD_3(, int, size_tToString, char*, destination, size_t, destinationSize, size_t, value)
    MOCK_METHOD_END(int, 0)

};

DECLARE_GLOBAL_MOCK_METHOD_0(CSASTokenMocks, , STRING_HANDLE, STRING_new);
DECLARE_GLOBAL_MOCK_METHOD_2(CSASTokenMocks, , int, STRING_concat, STRING_HANDLE, handle, const char*, s2);
DECLARE_GLOBAL_MOCK_METHOD_2(CSASTokenMocks, , int, STRING_concat_with_STRING, STRING_HANDLE, s1, STRING_HANDLE, s2);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , void, STRING_delete, STRING_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , size_t, STRING_length, STRING_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , const char*, STRING_c_str, STRING_HANDLE, s);
DECLARE_GLOBAL_MOCK_METHOD_2(CSASTokenMocks, , int, STRING_copy, STRING_HANDLE, handle, const char*, s2);


DECLARE_GLOBAL_MOCK_METHOD_0(CSASTokenMocks, , BUFFER_HANDLE, BUFFER_new);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , size_t, BUFFER_length, BUFFER_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , unsigned char*, BUFFER_u_char, BUFFER_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , void, BUFFER_delete, BUFFER_HANDLE, handle);

DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , STRING_HANDLE, Base64_Encode, BUFFER_HANDLE, input);
DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , BUFFER_HANDLE, Base64_Decoder, const char*, input);

DECLARE_GLOBAL_MOCK_METHOD_1(CSASTokenMocks, , STRING_HANDLE, URL_Encode, STRING_HANDLE, input);

DECLARE_GLOBAL_MOCK_METHOD_5(CSASTokenMocks, , HMACSHA256_RESULT, HMACSHA256_ComputeHash, const unsigned char*, key, size_t, keyLen, const unsigned char*, payload, size_t, payloadLen, BUFFER_HANDLE, hash);

DECLARE_GLOBAL_MOCK_METHOD_3(CSASTokenMocks, , int, size_tToString, char*, destination, size_t, destinationSize, size_t, value);

BEGIN_TEST_SUITE(sastoken_unittests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = MicroMockCreateMutex();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    MicroMockDestroyMutex(g_testByTest);
    DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (!MicroMockAcquireMutex(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    if (!MicroMockReleaseMutex(g_testByTest))
    {
        ASSERT_FAIL("failure in test framework at ReleaseMutex");
    }
}

/*Tests_SRS_SASTOKEN_06_001: [If key is NULL then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_null_key_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    // act
    handle = SASToken_Create(TEST_NULL_STRING_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_003: [If scope is NULL then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_null_scope_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_NULL_STRING_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_007: [If keyName is NULL then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_null_keyName_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;


    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_NULL_STRING_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_030: [If there is an error in the decoding then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_decoded_key_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_NULL_BUFFER_HANDLE);

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);

}

/*Tests_SRS_SASTOKEN_06_029: [The key parameter is decoded from base64.]*/
/*Tests_SRS_SASTOKEN_06_026: [If the conversion to string form fails for any reason then SASToken_Create shall return NULL.]*/
TEST_FUNCTION(SASToken_Create_size_tToString_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).SetReturn(1);

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_024: [The size_t value ((size_t) (difftime(get_time(NULL),0) + 3600)) is converted to a string form.]*/
TEST_FUNCTION(SASToken_Create_buffer_new_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME) ,0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_NULL_BUFFER_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_NULL_BUFFER_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

TEST_FUNCTION(SASToken_Create_first_string_new_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_NULL_STRING_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

TEST_FUNCTION(SASToken_Create_second_string_new_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_NULL_STRING_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

TEST_FUNCTION(SASToken_Create_build_to_be_hashed_part1_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE)).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_009: [The scope is the basis for creating a STRING_HANDLE.]*/
TEST_FUNCTION(SASToken_Create_build_to_be_hashed_part2_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n")).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_010: [A "\n" is appended to that string.]*/
TEST_FUNCTION(SASToken_Create_build_to_be_hashed_part3_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME)).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_011: [tokenExpirationTime is appended to that string.]*/
/*Tests_SRS_SASTOKEN_06_013: [If an error is returned from the HMAC256 function then NULL is returned from SASToken_Create.]*/
TEST_FUNCTION(SASToken_Create_HMAC256_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3).SetReturn(HMACSHA256_ERROR);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE)); // base64
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE)); // url signature

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}


/*Tests_SRS_SASTOKEN_06_012: [An HMAC256 hash is calculated using the decodedKey, over toBeHashed.]*/
/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
TEST_FUNCTION(SASToken_Create_HMAC256_passes_signature_encode_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_NULL_STRING_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE)); //base64
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE)); //url signature

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_015: [The hash is base 64 encoded.]*/
TEST_FUNCTION(SASToken_Create_building_token_signature_url_encoding_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_NULL_STRING_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_NULL_STRING_HANDLE));

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_028: [base64Signature shall be url encoded.]*/
TEST_FUNCTION(SASToken_Create_building_token_copy_scope_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr=")).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_016: [The string "SharedAccessSignature sr=" is the first part of the result of SASToken_Create.]*/
/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_scope_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE)).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_017: [The scope parameter is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_signature_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig=")).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_018: [The string "&sig=" is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_signature_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE)).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_019: [The string urlEncodedSignature shall be appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_token_expiration_time_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&se=")).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_020: [The string "&se=" shall be appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_token_expiration_time_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME)).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_021: [tokenExpirationTime is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_keyname_identifier_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&skn=")).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
/*Tests_SRS_SASTOKEN_06_022: [The string "&skn=" is appended to result.]*/
TEST_FUNCTION(SASToken_Create_building_token_concat_keyname_fails)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&skn="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_KEYNAME_HANDLE)).SetReturn(1);

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_RESULT_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NULL(handle);
}

/*Tests_SRS_SASTOKEN_06_023: [The argument keyName is appended to result.]*/
TEST_FUNCTION(SASToken_Create_succeeds)
{
    // arrange
    STRING_HANDLE handle;
    CSASTokenMocks mocks;

    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_KEY_HANDLE)).SetReturn(&TEST_CHAR_ARRAY[0]);
    STRICT_EXPECTED_CALL(mocks, Base64_Decoder(&TEST_CHAR_ARRAY[0])).SetReturn(TEST_DECODEDKEY_HANDLE);
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_DECODEDKEY_HANDLE));

    STRICT_EXPECTED_CALL(mocks, size_tToString(IGNORED_PTR_ARG, sizeof(TEST_TOKEN_EXPIRATION_TIME), TEST_EXPIRY)).IgnoreArgument(1).CopyOutArgumentBuffer(1, TEST_TOKEN_EXPIRATION_TIME, sizeof(TEST_TOKEN_EXPIRATION_TIME), 0);
    STRICT_EXPECTED_CALL(mocks, BUFFER_new()).SetReturn(TEST_HASH_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_TOBEHASHED_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_new()).SetReturn(TEST_RESULT_HANDLE);

    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_TOBEHASHED_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, "\n"));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_TOBEHASHED_HANDLE, TEST_TOKEN_EXPIRATION_TIME));

    STRICT_EXPECTED_CALL(mocks, BUFFER_u_char(TEST_DECODEDKEY_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_length(TEST_DECODEDKEY_HANDLE)).SetReturn(TEST_LENGTH_DECODEDKEY);
    STRICT_EXPECTED_CALL(mocks, STRING_c_str(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_length(TEST_TOBEHASHED_HANDLE)).SetReturn(TEST_LENGTH_TOBEHASHED);

    STRICT_EXPECTED_CALL(mocks, HMACSHA256_ComputeHash(IGNORED_PTR_ARG, TEST_LENGTH_DECODEDKEY, IGNORED_PTR_ARG, TEST_LENGTH_TOBEHASHED, TEST_HASH_HANDLE)).IgnoreArgument(1).IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mocks, Base64_Encode(TEST_HASH_HANDLE)).SetReturn(TEST_BASE64SIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_BASE64SIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, URL_Encode(TEST_BASE64SIGNATURE_HANDLE)).SetReturn(TEST_URLENCODEDSIGNATURE_HANDLE);
    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_copy(TEST_RESULT_HANDLE, "SharedAccessSignature sr="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_SCOPE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&sig="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_URLENCODEDSIGNATURE_HANDLE));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&se="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, TEST_TOKEN_EXPIRATION_TIME));
    STRICT_EXPECTED_CALL(mocks, STRING_concat(TEST_RESULT_HANDLE, "&skn="));
    STRICT_EXPECTED_CALL(mocks, STRING_concat_with_STRING(TEST_RESULT_HANDLE, TEST_KEYNAME_HANDLE));

    STRICT_EXPECTED_CALL(mocks, STRING_delete(TEST_TOBEHASHED_HANDLE));
    STRICT_EXPECTED_CALL(mocks, BUFFER_delete(TEST_HASH_HANDLE));

    // act
    handle = SASToken_Create(TEST_KEY_HANDLE, TEST_SCOPE_HANDLE, TEST_KEYNAME_HANDLE, TEST_EXPIRY);

    // assert
    ASSERT_IS_NOT_NULL(handle);
}

END_TEST_SUITE(sastoken_unittests)
