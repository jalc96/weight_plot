#include<windows.h>
#include "raylib.h"

#define al_min(a, b) ((a) < (b) ? (a) : (b))
#define al_max(a, b) ((a) < (b) ? (b) : (a))
#define al_clamp(n, min_n, max_n) al_min(al_max(n, min_n), max_n)

#include"types.h"
#include<stdio.h>
#include <math.h>
#include"debug.h"
#include"string.h"

global_variable Color BACKGROUND_COLOR = {64, 64, 64, 255};

template <class T> struct V2 {
    T x;
    T y;
};

template <class T> internal V2<f32> V2f(V2<T> v) {
    V2<f32> result;

    result.x = (f32)v.x;
    result.y = (f32)v.y;

    return result;
}

template <class T> internal V2<s32> V2s(V2<T> v) {
    V2<s32> result;

    result.x = (s32)v.x;
    result.y = (s32)v.y;

    return result;
}

template <class T> internal V2<T> operator -(V2<T> v) {
    V2<T> result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

template <class T> internal V2<T> operator +(V2<T> a, V2<T> b) {
    V2<T> result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

template <class T> internal V2<T> operator -(V2<T> a, V2<T> b) {
    V2<T> result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

template <class T> internal V2<T> operator *(V2<T> a, T k) {
    V2<T> result;
    result.x = a.x * k;
    result.y = a.y * k;
    return result;
}

template <class T> internal V2<T> operator /(V2<T> a, T k) {
    V2<T> result;
    result.x = a.x / k;
    result.y = a.y / k;
    return result;
}

template <class T> internal V2<T> hadamard(V2<T>a, V2<T> b) {
    V2<T> result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}

template <class T> inline V2<T> operator +=(V2<T> &b, V2<T> a) {
    b = a + b;
    return b;
}

template <class T> inline V2<T> operator -=(V2<T> &b, V2<T> a) {
    b = b - a;
    return b;
}

template <class T> internal V2<T> clamp(V2<T> v, V2<T> _min, V2<T> _max) {
    V2<T> result;

    result.x = al_clamp(v.x, _min.x, _max.x);
    result.y = al_clamp(v.y, _min.y, _max.y);

    return result;
}

template <class T> struct Rect2 {
    V2<T> min;
    V2<T> max;
};

template <class T> internal Rect2<T> add_radius(Rect2<T> rect, V2<T> radius) {
    Rect2<T> result = rect;

    result.min -= radius;
    result.max += radius;

    return result;
}

template <class T> internal V2<T> dim(Rect2<T> rect) {
    V2<T> result = rect.max - rect.min;
    return result;
}

template <class T> internal bool inside(Rect2<T> r, V2<T> p) {
    bool result = (
           p.x >= r.min.x
        && p.x < r.max.x
        && p.y >= r.min.y
        && p.y < r.max.y
    );
    return result;
}


internal str read_entire_file(char *file_path) {
    HANDLE file_handle = CreateFileA(
        file_path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    str result = {};

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER  size = {};
        GetFileSizeEx(file_handle, &size);

        result.count = (u64)size.LowPart;
        result.buffer = (char *)malloc(sizeof(char) * result.count);

        if (ReadFile(file_handle, (void *)result.buffer, (DWORD)result.count, NULL, NULL)) {
        } else {
            DWORD error = GetLastError();

            switch(error) {
                case ERROR_INSUFFICIENT_BUFFER: {
                    printf("\nERROR: buffer too small to hold the entire file, current file set to: %d\n", result.count);
                    break;
                };
                default: {
                    printf("\nERROR: unhandled error in read_entire_file reading the file, error code: %d\n", (s32)error);
                    break;
                };
            }

            result = {};
        }
        
        CloseHandle(file_handle);
    } else {
        DWORD error = GetLastError();

        switch(error) {
            case ERROR_FILE_NOT_FOUND: {
                printf("\nERROR: file '%s' not found\n", file_path);
                break;
            };
            case ERROR_ACCESS_DENIED: {
                printf("\nERROR: file '%s' access denied\n", file_path);
                break;
            };
            default: {
                printf("\nERROR: unhandled error in read_entire_file opening the file, error code: %d\n", (s32)error);
                break;
            };
        }
    }

    return result;
}

enum STATUS {
    NONE,
    DROPPING_FILES,
    SELECTING_FILES,
    DRAW_GRAPH,
};

enum MOUSE_MODE {
    MOUSE_CURSOR,
    MOUSE_INSPECT,
};

struct Raw_data {
    u32 count;
    u32 period = 1;
    f32 *data;
};

struct Linear_regression {
    f32 slope;
    f32 offset;
};

struct Stats {
    Linear_regression linear_regression;
    f32 min;
    f32 max;
    f32 avg;
};

internal Stats calculate_stats(f32 *data, u32 _count) {
    f32 count = (f32)_count;
    Stats result;
    result.min = F32_MAX;
    result.max = F32_MIN;
    result.avg = 0;
    f32 sum_x = 0.0f;
    f32 sum_y = 0.0f;
    f32 sum_x_sq = 0.0f;
    f32 sum_xy = 0.0f;

    for (u32 i = 0; i < _count; i++) {
        f32 it = data[i];
        result.min = al_min(it, result.min);
        result.max = al_max(it, result.max);
        sum_x    += (f32)i;
        sum_x_sq += (f32)(i * i);
        sum_y    += (f32)it;
        sum_xy   += (f32)i * it;
    }

    result.avg = sum_y / count;

    f32 d = (count * sum_x_sq) - (sum_x * sum_x);
    result.linear_regression.slope  = ((count * sum_xy)   - (sum_x * sum_y))  / d;
    result.linear_regression.offset = ((sum_y * sum_x_sq) - (sum_x * sum_xy)) / d;

    return result;
}

internal Raw_data group_by_period(Raw_data data, u32 period) {
    Raw_data result;
    u32 new_count = data.count / period;
    result.count = new_count;
    result.period = period;
    result.data = (f32 *)malloc(sizeof(f32) * new_count);

    for (u32 i = 0; i < new_count; i++) {
        f32 sum_period = 0.0f;

        for (u32 j = 0; j < period; j++) {
            u32 index = (i * period) + j;
            sum_period += data.data[index];
        }

        f32 avg_period = sum_period / (f32)period;
        result.data[i] = avg_period;
    }

    return result;
}

internal Raw_data slice(Raw_data original, u32 start, u32 count) {
    Raw_data result = original;
    result.count = count;
    result.data += start;
    return result;
}

struct Status {
    STATUS status;
    MOUSE_MODE mouse_mode;
    bool mouse_clicked;
    bool show_daily;
    bool show_weekly;
    bool show_monthly;
    bool show_vertical_region_separator;
    bool show_region_stats;

    bool by_day_set;
    Raw_data by_day;
    Raw_data by_week;
    Raw_data by_month;

    Stats stats_daily;
    Stats stats_weekly;
    Stats stats_monthly;

    s32 mouse_0_x0;
    s32 mouse_0_xf;
    s32 data_0_0;
    s32 data_0_f;

    s32 mouse_1_x0;
    s32 mouse_1_xf;
    s32 data_1_0;
    s32 data_1_f;

    bool show_selection;
    bool selection_0;
    bool selection_1;
    Raw_data mouse_selection_0;
    Raw_data mouse_selection_1;

    u32 filling_selection;

    u32 ii;
    u32 i_0[32];
    u32 i_f[32];

    SYSTEMTIME today;
    str last_file_path;
};

internal V2<s32> centered_text(str text, V2<s32> screen_dim, s32 text_size, Color color, s32 vertical_offset=0) {
    V2<s32> text_p;

    text_p.y = screen_dim.y / 3;
    text_p.y += vertical_offset;
    text_p.x = (screen_dim.x / 2) - (MeasureText(text.buffer, text_size) / 2);
    char t[1024];
    sprintf(t, "%.*s", text.count, text.buffer);
    rlDrawText(t, text_p.x, text_p.y, text_size, color);

    return text_p;
}

internal bool centered_button(str text, s32 x_dim, s32 y, s32 size, bool *status, bool *is_hot);

internal bool dropping_files(Status *status, V2<s32> screen_dim, str *result) {
    bool has_files = false;
    FilePathList dropped_files = {};

    if (IsFileDropped()) {
        // Is some files have been previously loaded, unload them
        if (dropped_files.count > 0) UnloadDroppedFiles(dropped_files);
        
        // Load new dropped files
        dropped_files = LoadDroppedFiles();
        has_files = true;
    }

    Color BACKGROUND_COLOR = {64, 64, 64, 255};
    s32 text_size = 60;

    BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        if (dropped_files.count == 0) {
            str drop_text = STATIC_STR("Drop your .w file to this window");
            str last_file_text = STATIC_STR("or select the last file:");
            centered_text(drop_text, screen_dim, text_size, RED);
            V2<s32> last_p = centered_text(last_file_text, screen_dim, 40, YELLOW, text_size);
            bool button_status = false;
            bool is_hot = false;

            if (centered_button(status->last_file_path, screen_dim.x, last_p.y + 40, 40, &button_status, &is_hot)) {
                *result = status->last_file_path;
                has_files = true;
            }
        } else {
            *result = STR(dropped_files.paths[0]);

            rlDrawText("Dropped files:", 100, 40, text_size, RED);

            for (int i = 0; i < dropped_files.count; i++) {
                if (i % 2 == 0) {
                    DrawRectangle(0, 85 + 40*i, screen_dim.x, 40, Fade(LIGHTGRAY, 0.5f));
                } else  {
                    DrawRectangle(0, 85 + 40*i, screen_dim.y, 40, Fade(LIGHTGRAY, 0.3f));
                }

                rlDrawText(dropped_files.paths[i], 120, 100 + 40*i, 10, GRAY);
            }

            rlDrawText("Drop new files...", 100, 110 + 40*dropped_files.count, text_size, DARKGRAY);
        }
    EndDrawing();

    return has_files;
}

struct Lexer {
    bool errors;
    str string;
    u32 index;
};

internal bool parsing(Lexer lexer) {
    bool result = (
           !lexer.errors
        && lexer.index < lexer.string.count
    );
    return result;
}

internal Lexer create_lexer(str string) {
    Lexer result;

    result.errors = false;
    result.string = string;
    result.index = 0;

    return result;
}

enum WTOKEN_TYPE {
    TOKEN_NONE,
    TOKEN_COMMENT,
    TOKEN_F32,
    TOKEN_EOL,
    TOKEN_EOF,


    TOKEN_COUNT,
};

struct Token  {
    WTOKEN_TYPE type;
    union {
        f32 f32_value;
        u32 u32_value;
        str str_value;
    };
};

internal char get_char(Lexer *lexer) {
    char result = lexer->string.buffer[lexer->index];
    return result;
}

internal void advance(Lexer *lexer) {
    lexer->index++;
}

internal f32 parse_float(Lexer *lexer) {
    char temp[8] = {};
    u32 i = 0;
    char c = get_char(lexer);

    while (is_numeric(c)) {
        temp[i++] = c;
        advance(lexer);
        c = get_char(lexer);
    }

    if (c == '.') {
        temp[i++] = c;
        advance(lexer);
        c = get_char(lexer);
    }

    while (is_numeric(c)) {
        temp[i++] = c;
        advance(lexer);
        c = get_char(lexer);
    }

    f32 result = to_float(temp);
    return result;
}

internal Token get_next_token(Lexer *lexer) {
    Token result;

    char c = get_char(lexer);

    if (lexer->index >= lexer->string.count) {
        result.type = TOKEN_EOF;
    } else if (is_numeric(c)) {
        result.type = TOKEN_F32;
        result.f32_value = parse_float(lexer);
    } else if (c == '/') {
        result.type = TOKEN_COMMENT;
        result.str_value = lexer->string;
        result.str_value.buffer += lexer->index;
        result.str_value.count = 0;

        while (c != '\n') {
            advance(lexer);
            c = get_char(lexer);
            result.str_value.count++;
        }
    } else if ((c == '\n') || (c == '\r')) {
        result.type = TOKEN_EOL;
        advance(lexer);
    } else {
        lexer->errors = true;
    }

    return result;
}

internal Raw_data parse_data(str file) {
    Raw_data result;
    result.count = 0;
    result.data = (f32 *)malloc(sizeof(f32) * 365 * 10);

    Lexer lexer = create_lexer(file);

    while (parsing(lexer)) {
        Token token = get_next_token(&lexer);

        if (token.type == TOKEN_F32) {
            result.data[result.count++] = token.f32_value;
        }
    }

    if (lexer.errors) {
        printf("error in parsing");
    }

    return result;
}


internal V2<s32> get_mouse_p(void) {
    V2<s32> result = {
        GetMouseX(),
        GetMouseY()
    };
    return result;
}

enum DATA_POINT_TYPE {
    DATA_POINT_DAILY,
    DATA_POINT_WEEKLY,
    DATA_POINT_MONTHLY,
    DATA_POINT_RANGE_0,
    DATA_POINT_RANGE_1,
};

internal void draw_data_points(Raw_data by_day, Raw_data data, f32 base_data, f32 time_period, V2<f32> draw_region_step, V2<f32> margin, V2<f32> screen_dim, DATA_POINT_TYPE type, bool show_vertical_region_separator, Stats stats, bool show_region_stats, s32 text_size, V2<s32> mouse, bool paint_data_points, u32 x_offset=0, u32 x_offset_selection=0) {
    f32 period_span = draw_region_step.x * time_period;
    f32 p0 = period_span / 2.0f;

    s32 width = 0;
    s32 height = 0;
    Color color = {};
    Color background = {};
    s32 sy0 = (s32)(screen_dim.y - margin.y);
    s32 sxf = (s32)(screen_dim.x - 2 * margin.x);
    s32 region_height = (s32)(screen_dim.y - 2.0f * margin.y);
    s32 slope_y = 0;
    s32 slope_x = 0;
    bool selected_region = false;

    switch (type) {
        case DATA_POINT_DAILY:     height = 2; background = color = RED;    background.a = 0;   slope_y = sy0 - (0 * (4 * text_size)); break;
        case DATA_POINT_WEEKLY:    height = 2; background = color = GREEN;  background.a = 0;   slope_y = sy0 - (1 * (4 * text_size)); break;
        case DATA_POINT_MONTHLY:   height = 4; background = color = BLUE;   background.a = 0;   slope_y = sy0 - (2 * (4 * text_size)); break;
        case DATA_POINT_RANGE_0:   height = 2; background = color = PURPLE; background.a = 80;  slope_y = sy0 - (3 * (4 * text_size)); selected_region = true; break;
        case DATA_POINT_RANGE_1:   height = 2; background = color = YELLOW; background.a = 80;  slope_y = sy0 - (4 * (4 * text_size)); selected_region = true; break;
    }

    // NOTE: the datapoint has 1 pixel as minimum width
    width = al_max(1, (s32)ceil(period_span));

    u32 i = 0;
    s32 index_to_slice = -1;
    s32 slice_x0 = -1;

    for (; i < data.count; i++) {
        V2<f32> data_space = {(f32)(i + x_offset) * time_period, data.data[i] - base_data};
        V2<f32> screen_space = hadamard(draw_region_step, data_space);
        s32 screen_x = (s32)(p0 + margin.x + screen_space.x);
        s32 screen_y = (s32)(screen_dim.y - (screen_space.y + margin.y));

        s32 region_x0 = (s32)(margin.x + (f32)i * period_span);
        s32 region_x1 = (s32)(margin.x + (f32)(i + 1) * period_span);

        // Draw the selection area. Very inefficient, but since the program doesnt do anything it doesnt matter
        if(paint_data_points) DrawRectangle(screen_x - width / 2, margin.y, width, region_height, background);

        // Draw the datapoint
        if(paint_data_points) DrawRectangle(screen_x - width / 2, screen_y, width, height, color);

        if (show_vertical_region_separator) {
            auto a = color.a;
            color.a = 80;
            if(paint_data_points) DrawRectangle(region_x0, (s32)margin.y, 1, region_height, color);
            color.a = a;
        }

        if (show_region_stats) {
            bool mouse_inside_region = (
                   ((mouse.x + margin.x) >= region_x0)
                && ((mouse.x + margin.x) < region_x1)
            );

            if (mouse_inside_region) {
                index_to_slice = i * data.period;
                slice_x0 = region_x0;
            }

            if (selected_region) {
                index_to_slice = i * data.period;
                slice_x0 = x_offset_selection * draw_region_step.x;
            }
        }
    }

    if (show_vertical_region_separator) {
        auto a = color.a;
        color.a = 80;
        if(paint_data_points) DrawRectangle((s32)(margin.x + (f32)i * period_span), (s32)margin.y, 1, region_height, color);
        color.a = a;
    }

    if ((!paint_data_points) && (index_to_slice >= 0)) {
        // Draw the stats in a floating text area
        Raw_data region_data = slice(by_day, index_to_slice, data.period);

        if (selected_region) {
            region_data = data;
        }

        Stats region_stats = calculate_stats(region_data.data, region_data.count);
        color.a = 190;
        s32 box_width = 0;

        char text_0[32];
        sprintf(text_0, "m: %.4f; b: %.2f", stats.linear_regression.slope, stats.linear_regression.offset);
        box_width = al_max(box_width, MeasureText(text_0, text_size));

        char text_1[32];
        sprintf(text_1, "min: %.2f", region_stats.min);
        box_width = al_max(box_width, MeasureText(text_1, text_size));

        char text_2[32];
        sprintf(text_2, "max: %.2f", region_stats.max);
        box_width = al_max(box_width, MeasureText(text_2, text_size));

        char text_3[32];
        sprintf(text_3, "avg: %.2f", region_stats.avg);
        box_width = al_max(box_width, MeasureText(text_3, text_size));


        slope_x = slice_x0;
        s32 out_of_region = al_max(0, (slope_x + box_width) - sxf);
        slope_x -= out_of_region;

        for (u32 j = 0; j < by_day.count; j++) {
            // If any of the points overlaps the text area, place the text area in a different place (400 pixels avobe)
            V2<f32> data_space = {(f32)j, by_day.data[j] - base_data};
            V2<f32> screen_space = hadamard(draw_region_step, data_space);
            f32 p0 = 1.0f / 2.0f;
            s32 screen_x = (s32)(p0 + margin.x + screen_space.x);
            s32 screen_y = (s32)(screen_dim.y - (screen_space.y + margin.y));

            bool point_inside_rectangle = (
                   screen_x >= slope_x
                && screen_x <= (slope_x + box_width)
                && screen_y >= slope_y
                && screen_y <= (slope_y + (4 * text_size))
            );

            if (point_inside_rectangle) {
                slope_y -= 400;
            }
        }

        DrawRectangle(slope_x, slope_y, box_width, 4 * text_size, color);

        rlDrawText(text_0, slope_x, slope_y + 0 * text_size, text_size, BLACK);
        rlDrawText(text_1, slope_x, slope_y + 1 * text_size, text_size, BLACK);
        rlDrawText(text_2, slope_x, slope_y + 2 * text_size, text_size, BLACK);
        rlDrawText(text_3, slope_x, slope_y + 3 * text_size, text_size, BLACK);

        color.a = 255;
    }
}

internal bool centered_button(str text, s32 x_dim, s32 y, s32 size, bool *status, bool *is_hot) {
    V2<s32> mouse_p = get_mouse_p();
    Color color = RED;
    s32 text_measure = MeasureText(text.buffer, size);
    V2<s32> p_text = {
        (x_dim / 2) - (text_measure / 2),
        y
    };
    s32 margin = 2;

    Rect2<s32> area = {
        {p_text.x, p_text.y},
        {p_text.x + text_measure, p_text.y + size}
    };
    add_radius(area, {2, 2});

    V2<s32> dimension = {
        area.max.x - area.min.x,
        area.max.y - area.min.y
    };
    char t[1024];
    sprintf(t, "%.*s", text.count, text.buffer);

    if (inside(area, mouse_p)) {
        *is_hot = true;
        color = WHITE;
        DrawRectangle(area.min.x, area.min.y, dimension.x, dimension.y, color);
        rlDrawText(t, p_text.x, p_text.y, size, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *status = !*status;
        }
    } else {
        DrawRectangle(area.min.x, area.min.y, dimension.x, dimension.y, color);
        rlDrawText(t, p_text.x, p_text.y, size, BLACK);
    }

    return *status;
}

internal bool checkbox(str text, V2<s32> p, s32 size, bool *status, bool *is_hot) {
    V2<s32> mouse_p = get_mouse_p();
    Color color = GREEN;
    V2<s32> p_text = p;
    s32 margin = 2;
    p_text.x += size + margin;

    Rect2<s32> area = {
        {p.x, p.y},
        {p_text.x + MeasureText(text.buffer, size), p_text.y + size}
    };

    if (inside(area, mouse_p)) {
        *is_hot = true;
        color = WHITE;
        DrawRectangle(p.x, p.y, size - 2, size - 2, color);
        DrawRectangle(p.x + 2, p.y + 2, size - 6, size - 6, BACKGROUND_COLOR);

        if (*status) DrawRectangle(p.x + 4, p.y + 4, size - 10, size - 10, color);

        rlDrawText(text.buffer, p_text.x, p_text.y, size, color);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *status = !*status;
        }
    } else {
        DrawRectangle(p.x, p.y, size - 2, size - 2, color);
        DrawRectangle(p.x + 2, p.y + 2, size - 6, size - 6, BACKGROUND_COLOR);

        if (*status) DrawRectangle(p.x + 4, p.y + 4, size - 10, size - 10, color);

        rlDrawText(text.buffer, p_text.x, p_text.y, size, color);
    }

    return *status;
}

global_variable u64 day_delta = 0;

internal u64 get_1_day_delta() {
    SYSTEMTIME today;
    GetSystemTime(&today);
    FILETIME today_in_filetime;
    FILETIME day_after;
    today.wDay = 1;
    SystemTimeToFileTime(&today, &today_in_filetime);
    today.wDay = 2;
    u64 today_in_u64 = (
          (u64)today_in_filetime.dwLowDateTime
        | (u64)today_in_filetime.dwHighDateTime << 32
    );
    SystemTimeToFileTime(&today, &day_after);
    u64 day_after_in_u64 = (
          (u64)day_after.dwLowDateTime
        | (u64)day_after.dwHighDateTime << 32
    );
    u64 result = day_after_in_u64 - today_in_u64;
    return result;
}

internal SYSTEMTIME subtract(SYSTEMTIME day, u32 count) {
    FILETIME day_in_filetime;
    SystemTimeToFileTime(&day, &day_in_filetime);
    u64 day_in_u64 = (
          (u64)day_in_filetime.dwLowDateTime
        | (u64)day_in_filetime.dwHighDateTime << 32
    );
    day_in_u64 -= (u64)count * day_delta;
    FILETIME temp;
    temp.dwLowDateTime = (u32)day_in_u64;
    temp.dwHighDateTime = (u32)(day_in_u64 >> 32);
    SYSTEMTIME result;
    FileTimeToSystemTime(&temp, &result);
    return result;
}

internal void track_selection(Status *status, V2<s32> safe_probe, s32 data_space_probe_x) {
    if (status->filling_selection == 0) {
        status->selection_0 = true;
        status->mouse_0_xf = safe_probe.x;
        status->data_0_f = data_space_probe_x;

        s32 min = al_min(status->data_0_0, status->data_0_f);
        s32 max = al_max(status->data_0_0, status->data_0_f);

        status->mouse_selection_0 = slice(status->by_day, min, max - min + 1);
    } else if (status->filling_selection == 1) {
        status->selection_1 = true;
        status->mouse_1_xf = safe_probe.x;
        status->data_1_f = data_space_probe_x;

        s32 min = al_min(status->data_1_0, status->data_1_f);
        s32 max = al_max(status->data_1_0, status->data_1_f);

        status->mouse_selection_1 = slice(status->by_day, min, max - min + 1);
    }
}

internal void draw_data(Status *status, V2<f32> screen_dim) {
    Raw_data by_day   = status->by_day;
    Raw_data by_week  = status->by_week;
    Raw_data by_month = status->by_month;

    Stats stats_daily   = status->stats_daily;
    Stats stats_weekly  = status->stats_weekly;
    Stats stats_monthly = status->stats_monthly;

    V2<f32> margin = {20.0f, 40.0f};
    Rect2<f32> draw_region = {{0.0f, 0.0f}, screen_dim};
    draw_region = add_radius(draw_region, -margin);
    Rect2<s32> sdraw_region = {V2s(draw_region.min), V2s(draw_region.max)};

    V2<f32> draw_region_dim = dim(draw_region);
    V2<f32> data_space_span = {
        (f32)by_day.count,
        stats_daily.max - stats_daily.min
    };

    V2<f32> draw_region_step = {
        draw_region_dim.x / data_space_span.x,
        draw_region_dim.y / data_space_span.y
    };

    s32 text_size = 20;

    V2<s32> mouse_p = get_mouse_p();
    V2<s32> safe_probe = clamp(mouse_p, sdraw_region.min, sdraw_region.max);
    s32 m_h_y = safe_probe.y;
    s32 m_v_x = safe_probe.x;
    V2<s32> sdraw_region_d = dim(sdraw_region);
    safe_probe.x = safe_probe.x - sdraw_region.min.x;
    safe_probe.y = sdraw_region.max.y - safe_probe.y;
    s32 data_space_probe_x = (s32)((f32)safe_probe.x / draw_region_step.x);
    f32 data_space_probe_y = (f32)safe_probe.y / draw_region_step.y;
    f32 weight_in_cursor = stats_daily.min + data_space_probe_y;


    SYSTEMTIME today = status->today;
    bool checkboxes_are_hot = false;
    
    BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        if (status->mouse_mode == MOUSE_CURSOR) {
            checkbox(            STR("daily"), {10, text_size * 1}, text_size, &status->show_daily, &checkboxes_are_hot);
            checkbox(           STR("weekly"), {10, text_size * 2}, text_size, &status->show_weekly, &checkboxes_are_hot);
            checkbox(          STR("monthly"), {10, text_size * 3}, text_size, &status->show_monthly, &checkboxes_are_hot);
            checkbox(      STR("show_region"), {10, text_size * 4}, text_size, &status->show_vertical_region_separator, &checkboxes_are_hot);
            checkbox(STR("show_region_stats"), {10, text_size * 5}, text_size, &status->show_region_stats, &checkboxes_are_hot);
        }

        if ((!checkboxes_are_hot) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            status->show_selection = false;
            status->selection_0 = false;
            status->selection_1 = false;
            status->mouse_0_x0 = 0;
            status->mouse_0_xf = 0;
            status->data_0_0 = 0;
            status->data_0_f = 0;
            status->mouse_1_x0 = 0;
            status->mouse_1_xf = 0;
            status->data_1_0 = 0;
            status->data_1_f = 0;
            status->filling_selection = 0;
        }

        if ((!checkboxes_are_hot) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            status->show_selection = true;
            status->filling_selection = (s32)IsKeyDown(KEY_LEFT_SHIFT);

            if (status->filling_selection == 0) {
                status->mouse_0_x0 = safe_probe.x;
                status->data_0_0 = data_space_probe_x;
            } else if (status->filling_selection == 1) {
                status->mouse_1_x0 = safe_probe.x;
                status->data_1_0 = data_space_probe_x;
            }
        }

        if ((!checkboxes_are_hot) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            track_selection(status, safe_probe, data_space_probe_x);
        }

        if ((!checkboxes_are_hot) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            track_selection(status, safe_probe, data_space_probe_x);
        }

        status->show_region_stats &= (status->mouse_mode == MOUSE_CURSOR);
        status->show_selection &= (status->mouse_mode == MOUSE_CURSOR);

        // First draw the datapoints
        if (status->show_monthly) {
            draw_data_points(by_day, by_month, stats_daily.min, 28.0f, draw_region_step, margin, screen_dim, DATA_POINT_MONTHLY, status->show_vertical_region_separator, stats_monthly, status->show_region_stats, text_size, safe_probe, true);
        }

        if (status->show_weekly) {
            draw_data_points(by_day, by_week, stats_daily.min, 7.0f, draw_region_step, margin, screen_dim, DATA_POINT_WEEKLY, status->show_vertical_region_separator, stats_weekly, status->show_region_stats, text_size, safe_probe, true);
        }

        if (status->show_daily) {
            draw_data_points(by_day, by_day, stats_daily.min, 1.0f, draw_region_step, margin, screen_dim, DATA_POINT_DAILY, false, stats_daily, false, text_size, safe_probe, true);
        }

        if (status->show_selection) {
            if (status->selection_0) {
                Stats stats_selection_0 = calculate_stats(status->mouse_selection_0.data, status->mouse_selection_0.count);
                draw_data_points(by_day, status->mouse_selection_0, stats_daily.min, 1.0f, draw_region_step, margin, screen_dim, DATA_POINT_RANGE_0, false, stats_selection_0, true, text_size, safe_probe, true, al_min(status->data_0_0, status->data_0_f), al_max(status->data_0_0, status->data_0_f));
            }

            if (status->selection_1) {
                Stats stats_selection_1 = calculate_stats(status->mouse_selection_1.data, status->mouse_selection_1.count);
                draw_data_points(by_day, status->mouse_selection_1, stats_daily.min, 1.0f, draw_region_step, margin, screen_dim, DATA_POINT_RANGE_1, false, stats_selection_1, true, text_size, safe_probe, true, al_min(status->data_1_0, status->data_1_f), al_max(status->data_1_0, status->data_1_f));
            }
        }


        // Then draw the textboxes
        if (status->show_monthly) {
            draw_data_points(by_day, by_month, stats_daily.min, 28.0f, draw_region_step, margin, screen_dim, DATA_POINT_MONTHLY, status->show_vertical_region_separator, stats_monthly, status->show_region_stats, text_size, safe_probe, false);
        }

        if (status->show_weekly) {
            draw_data_points(by_day, by_week, stats_daily.min, 7.0f, draw_region_step, margin, screen_dim, DATA_POINT_WEEKLY, status->show_vertical_region_separator, stats_weekly, status->show_region_stats, text_size, safe_probe, false);
        }

        if (status->show_daily) {
            draw_data_points(by_day, by_day, stats_daily.min, 1.0f, draw_region_step, margin, screen_dim, DATA_POINT_DAILY, false, stats_daily, false, text_size, safe_probe, false);
        }

        if (status->show_selection) {
            if (status->selection_0) {
                Stats stats_selection_0 = calculate_stats(status->mouse_selection_0.data, status->mouse_selection_0.count);
                draw_data_points(by_day, status->mouse_selection_0, stats_daily.min, 1.0f, draw_region_step, margin, screen_dim, DATA_POINT_RANGE_0, false, stats_selection_0, true, text_size, safe_probe, false, al_min(status->data_0_0, status->data_0_f), al_max(status->data_0_0, status->data_0_f));
            }

            if (status->selection_1) {
                Stats stats_selection_1 = calculate_stats(status->mouse_selection_1.data, status->mouse_selection_1.count);
                draw_data_points(by_day, status->mouse_selection_1, stats_daily.min, 1.0f, draw_region_step, margin, screen_dim, DATA_POINT_RANGE_1, false, stats_selection_1, true, text_size, safe_probe, false, al_min(status->data_1_0, status->data_1_f), al_max(status->data_1_0, status->data_1_f));
            }
        }


        // If inspect mode draw the vertical and horizontal lines and the textbox probe
        if (status->mouse_mode == MOUSE_INSPECT) {
            SetMousePosition(m_v_x, m_h_y);
            DrawRectangle(0, m_h_y, (s32)screen_dim.x, 1, WHITE);
            DrawRectangle(m_v_x, 0, 1, (s32)screen_dim.y, WHITE);

            char w_msg[32];
            sprintf(w_msg, "%.2f kg", weight_in_cursor);
            s32 msg_width = MeasureText(w_msg, text_size);


            SYSTEMTIME day_queried = subtract(today, by_day.count - data_space_probe_x);
            char d_msg[32];
            sprintf(d_msg, "%d/%d/%d", day_queried.wYear, day_queried.wMonth, day_queried.wDay);
            msg_width = al_max(msg_width, MeasureText(d_msg, text_size));

            V2<s32> offset = {40, 5};
            V2<s32> weight_p = {m_v_x, m_h_y};
            weight_p += offset;

            if ((weight_p.x + msg_width) > draw_region.max.x) {
                weight_p.x -= 2 * offset.x + msg_width;
            }

            if ((weight_p.y + (2 * text_size)) > draw_region.max.y) {
                weight_p.y -= 2 * (offset.y + text_size);
            }

            DrawRectangle(weight_p.x, weight_p.y, msg_width,  2 * text_size, {0, 0, 0, 150});
            rlDrawText(w_msg, weight_p.x, weight_p.y, text_size, GREEN);
            rlDrawText(d_msg, weight_p.x, weight_p.y + text_size, text_size, GREEN);
        }

        DrawFPS(1800, 0);
    EndDrawing();
}

// Very crappy file handling
internal bool data_file_is_present() {
    FILE *file = fopen("info.data", "r");
    bool result = false;

    if (file) {
        result = true;
        fclose(file);
    }

    return result;
}

internal str get_last_file_path() {
    str result = read_entire_file("info.data");
    return result;
}

internal void create_data_file() {
    FILE *file = fopen("info.data", "w");

    if (file) {
        fclose(file);
    }
}

internal void save_data_file(Status *status) {
    FILE *file = fopen("info.data", "w");
    str lfp = status->last_file_path;

    if (file) {
        fwrite(lfp.buffer, sizeof(char), lfp.count + 1, file);
        fclose(file);
    }
}

char path[65536];

int main(void) {
    // WARNING (Windows): If program is compiled as Window application (instead of console),
    // no console is available to show output info... solution is compiling a console application
    // and closing console (FreeConsole()) when changing to GUI interface
    FreeConsole();

    V2<s32> screen_dim = {1920, 1080};

    InitWindow(screen_dim.x, screen_dim.y, "weight");

    day_delta = get_1_day_delta();

    SetTargetFPS(60);
    Status status = {};
    status.show_daily = true;
    status.show_weekly = true;
    status.show_monthly = true;
    status.by_day_set = false;
    status.status = DROPPING_FILES;
    status.mouse_mode = MOUSE_CURSOR;
    status.selection_0 = false;
    status.selection_1 = false;
    status.show_selection = false;
    status.show_vertical_region_separator = true;
    status.show_region_stats = true;

    if (data_file_is_present()) {
        status.last_file_path = get_last_file_path();
    } else {
        create_data_file();
        status.last_file_path = {};
    }

    str file_path = {};

    while (!WindowShouldClose() && status.status != NONE) {
        switch (status.status) {
            case DROPPING_FILES: {
                rlShowCursor();

                bool has_file = dropping_files(&status, screen_dim, &file_path);

                if (has_file) {
                    status.status = DRAW_GRAPH;
                }
            } break;

            case SELECTING_FILES: {
                status.status = NONE;
            } break;

            case DRAW_GRAPH: {
                if (status.mouse_mode == MOUSE_CURSOR) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        HideCursor();
                        status.mouse_mode = MOUSE_INSPECT;
                    }
                } else if (status.mouse_mode == MOUSE_INSPECT) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        rlShowCursor();
                        status.mouse_mode = MOUSE_CURSOR;
                    }
                }
    
                if (!status.by_day_set) {
                    str file;

                    char _file_path[1024];
                    sprintf(_file_path, "%.*s", file_path.count, file_path.buffer);

                    file = read_entire_file(_file_path);

                    status.last_file_path = STR(_file_path);

                    status.by_day = parse_data(file);
                    status.by_day_set = true;
                    status.by_week = group_by_period(status.by_day, 7);
                    status.by_month = group_by_period(status.by_day, 28);
                    status.stats_daily   = calculate_stats(status.by_day.data, status.by_day.count);
                    status.stats_weekly  = calculate_stats(status.by_week.data, status.by_week.count);
                    status.stats_monthly = calculate_stats(status.by_month.data, status.by_month.count);

                    GetSystemTime(&status.today);
                }

                draw_data(&status, V2f(screen_dim));
            } break;
        }

    }

    rlCloseWindow();

    if (!data_file_is_present()) {
        create_data_file();
    }

    save_data_file(&status);

    return 0;
}