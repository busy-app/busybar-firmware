#include "path.h"

#include <core/check.h>

void path_extract_filename_no_ext(const char* path, FuriString* filename) {
    furi_check(path);
    furi_check(filename);

    furi_string_set(filename, path);

    size_t start_position = furi_string_search_rchar(filename, '/');
    size_t end_position = furi_string_search_rchar(filename, '.');

    if(start_position == FURI_STRING_FAILURE) {
        start_position = 0;
    } else {
        start_position += 1;
    }

    if(end_position == FURI_STRING_FAILURE) {
        end_position = furi_string_size(filename);
    }

    furi_string_mid(filename, start_position, end_position - start_position);
}

void path_extract_filename(FuriString* path, FuriString* name, bool trim_ext) {
    furi_check(path);
    furi_check(name);

    size_t filename_start = furi_string_search_rchar(path, '/');
    if(filename_start > 0) {
        filename_start++;
        furi_string_set_n(name, path, filename_start, furi_string_size(path) - filename_start);
    }
    if(trim_ext) {
        size_t dot = furi_string_search_rchar(name, '.');
        if(dot > 0) {
            furi_string_left(name, dot);
        }
    }
}

void path_extract_extension(FuriString* path, char* ext, size_t ext_len_max) {
    furi_check(path);
    furi_check(ext);
    furi_check(ext_len_max > 0);

    size_t dot = furi_string_search_rchar(path, '.');
    size_t filename_start = furi_string_search_rchar(path, '/');

    if((dot != FURI_STRING_FAILURE) && (filename_start < dot)) {
        strlcpy(ext, &(furi_string_get_cstr(path))[dot], ext_len_max);
    }
}

static inline void path_cleanup(FuriString* path) {
    furi_string_trim(path);
    while(furi_string_end_with(path, "/")) {
        furi_string_left(path, furi_string_size(path) - 1);
    }
}

void path_extract_basename(const char* path, FuriString* basename) {
    furi_check(path);
    furi_check(basename);

    furi_string_set(basename, path);
    path_cleanup(basename);
    size_t pos = furi_string_search_rchar(basename, '/');
    if(pos != FURI_STRING_FAILURE) {
        furi_string_right(basename, pos + 1);
    }
}

void path_extract_dirname(const char* path, FuriString* dirname) {
    furi_check(path);
    furi_check(dirname);

    furi_string_set(dirname, path);
    path_cleanup(dirname);
    size_t pos = furi_string_search_rchar(dirname, '/');
    if(pos != FURI_STRING_FAILURE) {
        furi_string_left(dirname, pos);
    }
}

void path_append(FuriString* path, const char* suffix) {
    furi_check(path);
    furi_check(suffix);

    path_cleanup(path);
    FuriString* suffix_str;
    suffix_str = furi_string_alloc_set(suffix);
    furi_string_trim(suffix_str);
    furi_string_trim(suffix_str, "/");
    furi_string_cat_printf(path, "/%s", furi_string_get_cstr(suffix_str));
    furi_string_free(suffix_str);
}

void path_concat(const char* path, const char* suffix, FuriString* out_path) {
    furi_check(path);
    furi_check(suffix);
    furi_check(out_path);

    furi_string_set(out_path, path);
    path_append(out_path, suffix);
}

bool path_contains_only_ascii(const char* path) {
    if(!path) {
        return false;
    }

    const char* name_pos = strrchr(path, '/');
    if(name_pos == NULL) {
        name_pos = path;
    } else {
        name_pos++;
    }

    for(; *name_pos; ++name_pos) {
        const char c = *name_pos;

        // Regular ASCII characters from 0x20 to 0x7e
        const bool is_out_of_range = (c < ' ') || (c > '~');
        // Cross-platform forbidden character set
        const bool is_forbidden = strchr("\\<>*|\":?", c);

        if(is_out_of_range || is_forbidden) {
            return false;
        }
    }

    return true;
}

static size_t count_path_separators(const char* path) {
    size_t result = 0;
    for(; *path; ++path) {
        if(*path == '/') {
            result += 1;
        }
    }
    return result;
}

static void join_with_path_separator(FuriString* a, const char* b) {
    size_t len = furi_string_size(a);
    if(len == 0) {
        furi_string_set(a, b);
    } else {
        if(furi_string_get_char(a, len - 1) != '/') {
            furi_string_cat(a, "/");
        }
        furi_string_cat(a, b);
    }
}

void path_normalize(const char* path, FuriString* out_path, bool allow_escape_root) {
    bool is_abs_path = path[0] == '/';
    if(is_abs_path) {
        ++path;
    }

    char* own_path = malloc(strlen(path) + 1);
    strcpy(own_path, path);
    size_t max_num_fragments = count_path_separators(path) + 1;
    const char** fragments = malloc(sizeof(char*) * (max_num_fragments + 1));
    size_t num_parents = 0;
    {
        char* lasts = NULL;
        size_t fragment_i = 0;
        for(const char* fragment = strtok_r(own_path, "/", &lasts); fragment;
            fragment = strtok_r(NULL, "/", &lasts)) {
            if(strcmp(fragment, ".") == 0) {
                // skip
            } else if(strcmp(fragment, "..") == 0) {
                if(fragment_i == 0) {
                    // already at toplevel
                    num_parents += 1;
                } else {
                    --fragment_i;
                    fragments[fragment_i] = NULL;
                }
            } else {
                fragments[fragment_i] = fragment;
                ++fragment_i;
            }
        }
    }

    if(is_abs_path) {
        furi_string_set(out_path, "/");
    }
    if(!is_abs_path && allow_escape_root) {
        for(size_t i = 0; i != num_parents; ++i) {
            join_with_path_separator(out_path, "..");
        }
    }

    for(const char** fragment = fragments; *fragment; ++fragment) {
        join_with_path_separator(out_path, *fragment);
    }

    free(own_path);
    free(fragments);
}
