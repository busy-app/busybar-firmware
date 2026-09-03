/**
 * @file api_tokens.h
 * 
 * Manages HTTP access tokens
 */

#pragma once

#include <furi.h>
#include <time.h>

#define RECORD_API_TOKENS          "api_tokens"
#define API_TOKENS_LENGTH          (32)
#define API_TOKENS_SHORT_ID_LENGTH (8)

typedef struct ApiTokens ApiTokens;

typedef enum {
    ApiApiTokensEntryTypeHashed,
    ApiApiTokensEntryTypeFull,
    ApiApiTokensEntryTypeMax,
} ApiApiTokensEntryType;

/**
 * @brief Information about an HTTP API access token
 */
typedef struct {
    ApiApiTokensEntryType type;
    char* short_id;
    char* display_id;
    char* owner;
    union {
        char* full_token; // <! `TypeFull`
        char* token_hash; // <! `TypeHashed`
    };
    time_t created_at;
    time_t last_used_at;
} ApiTokensEntry;

typedef void (*ApiTokenInfoCallback)(const ApiTokensEntry* entry, void* context);

/**
 * @brief Generates a new HTTP API access token.
 * 
 * @param[inout] tokens ApiTokens service
 * @param[in] owner Human-readable name for the token
 * @param[in] callback Callback with info about the generated token
 * @param[in] context Custom context to pass to callback verbatim
 */
void api_tokens_mint(
    ApiTokens* tokens,
    const char* owner,
    ApiTokenInfoCallback callback,
    void* context);

/**
 * @brief List all HTTP API access tokens
 * 
 * @param[in] tokens ApiTokens service
 * @param[in] callback Callback, called for each listed token
 * @param[in] context Custom context to pass to callback verbatim
 */
void api_tokens_list(ApiTokens* tokens, ApiTokenInfoCallback callback, void* context);

/**
 * @brief Revoke one HTTP API access token
 * 
 * @param[inout] tokens ApiTokens service
 * @param[in] short_id Token short ID
 * 
 * @returns `false` if requested token was not found
 */
bool api_tokens_revoke(ApiTokens* tokens, const char* short_id);

/**
 * @brief Revoke all HTTP API access tokens
 * 
 * @param[inout] tokens ApiTokens service
 */
void api_tokens_reset_all(ApiTokens* tokens);

/**
 * @brief Check that the provided access token is valid, and update its
 * `last_used_at` property
 * 
 * @param[inout] tokens ApiTokens service
 * @param[in] full_token Full access token
 * 
 * @returns `true` if access should be granted
 */
bool api_tokens_validate_and_record_usage(ApiTokens* tokens, const char* full_token);
