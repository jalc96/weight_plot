struct str {
    u32 count;
    char *buffer;
};

#if !defined(static_length)
    #define static_length(array) (sizeof(array) / sizeof((array)[0]))
#endif

#if !defined(assert)
    #define assert(...)
#endif

#define str_for(string) char it = string.buffer[0]; for (u32 i = 0; i < string.count; i++, it = string.buffer[i])
#define STATIC_STR(static_zero_terminated_string) {static_length(static_zero_terminated_string) - 1, static_zero_terminated_string}
#define STR(zero_terminated_string) {length(zero_terminated_string), zero_terminated_string}


internal u32 to_int(char c) {
    u32 result = c - '0';
    return result;
}


#define isdigit(c) (c >= '0' && c <= '9')

double to_float(const char *s) {
    // https://github.com/GaloisInc/minlibc/blob/master/atof.c
    // look for ascii to float transformation and use a good implementation, i dont know if this code is a good implementation
  // This function stolen from either Rolf Neugebauer or Andrew Tolmach. 
  // Probably Rolf.
  double a = 0.0;
  int e = 0;
  int c;
  while ((c = *s++) != '\0' && isdigit(c)) {
    a = a*10.0 + (c - '0');
  }
  if (c == '.') {
    while ((c = *s++) != '\0' && isdigit(c)) {
      a = a*10.0 + (c - '0');
      e = e-1;
    }
  }
  if (c == 'e' || c == 'E') {
    int sign = 1;
    int i = 0;
    c = *s++;
    if (c == '+')
      c = *s++;
    else if (c == '-') {
      c = *s++;
      sign = -1;
    }
    while (isdigit(c)) {
      i = i*10 + (c - '0');
      c = *s++;
    }
    e += i*sign;
  }
  while (e > 0) {
    a *= 10.0;
    e--;
  }
  while (e < 0) {
    a *= 0.1;
    e++;
  }
  return a;
}



internal bool is_eol(char c) {
    return (c == '\n');
}

internal bool is_whitespace(char c) {
    if (c == ' ') return true;
    if (c == '\t') return true;
    if (c == '\r') return true;
    return false;
}

internal bool is_alpha(char c) {
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    return false;
}

internal bool is_numeric(char c) {
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

internal bool is_alpha_numeric(char c) {
    return (is_alpha(c) || is_numeric(c));
}

internal void copy(char *src, char *dst) {
    while (*src) {
        *dst++ = *src++;
    }
}

internal u32 length(char *buffer) {
    u64 c0 = (u64)buffer;

    while (*buffer) {
        buffer++;
    }

    u32 result = (u32)((u64)buffer - c0);
    return result;
}

internal bool match(str origin, str pattern) {
    // assert(origin.count >= pattern.count, "pattern in string match must be smaller than the origin");
    u32 i = 0;

    while ((i < origin.count) && (i < pattern.count)) {
        if (origin.buffer[i] != pattern.buffer[i]) {
            return false;
        }

        i++;
    }

    return true;
}

internal str create_str(char *buffer) {
    str result;
    result.buffer = buffer;
    result.count = length(buffer);
    return result;
};

internal u32 length(str string) {
    return string.count;
}

internal u32 length(str *string) {
    return string->count;
}

internal void offset(str *string, u32 i) {
    assert(i <= string->count, "cannot offset an string more than its size");
    string->buffer += i;
    string->count -= i;
}


// Hash Functions
// http://www.cse.yorku.ca/~oz/hash.html

internal u32 hash(str string) {
    // djb2
    u32 hash = 5381;

    str_for (string) {
        hash = ((hash << 5) + hash) + it; /* hash * 33 + it */
    }

    return hash;
}

internal u32 hash2(str string) {
    // sdbm
    u32 hash = 0;

    str_for (string) {
        hash = it + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

internal u32 hash3(str string) {
    // lose lose
    u32 hash = 0;

    str_for (string) {
        hash += it;
    }

    return hash;
}

