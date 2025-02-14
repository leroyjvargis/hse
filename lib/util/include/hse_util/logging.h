/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2021 Micron Technology, Inc.  All rights reserved.
 */

#ifndef HSE_PLATFORM_LOGGING_H
#define HSE_PLATFORM_LOGGING_H

#include <hse_util/arch.h>
#include <hse_util/inttypes.h>
#include <hse_util/compiler.h>
#include <hse_util/data_tree.h>
#include <hse_util/event_counter.h>
#include <hse_util/logging_types.h>

#include <syslog.h>

/* clang-format off */

#ifndef HSE_LOGPRI_DEFAULT
#define HSE_LOGPRI_DEFAULT  (HSE_LOGPRI_DEBUG)
#endif

/* The mark is prepended to every outgoing log message issued via
 * the non-structured logging calls (e.g., log_info()), but not
 * the structure log calls (e.g., slog_info()).
 */
#ifndef HSE_MARK
#define HSE_MARK            "[HSE]"
#endif

/*
 * A single log instance can have no more than MAX_HSE_SPECS hse-specific
 * conversion specifiers. For that instance there can be no more than
 * HSE_LOG_NV_PAIRS_MAX of structured log data entries created.
 *
 * As structured name value data is accumulated it must be retained
 * until right before the dynamic call to json payload formatter so that
 * those values can be tacked onto its argument list.
 */
#define HSE_LOG_SPECS_MAX   (10)

/*
 * A single log instance can have no more than HSE_LOG_NV_PAIRS_MAX of structured
 * log data entries created.
 *
 * The need for the space is due to current logging logic that requies the
 * data to be accumulated and passed as a argument to json formatter to
 * generate a json formated payload which is passed as a final argument
 * to syslog in user space or printk_emit in case of kernel.
 * As structured name value data is accumulated it must be retained until
 * those values can be tacked onto its argument list.
 */
#define HSE_LOG_NV_PAIRS_MAX  (40 * HSE_LOG_SPECS_MAX)

/*
 * The HSE platform logging subsystem needs to accept client-registered
 * conversion specifiers.
 *
 * The HSE platform logging subsystem defines a set of HSE-specific conversion
 * specifiers.  For example, one can give "The error: @@e" as a format string
 * to hse_xlog and pass in a pointer to an hse_err_t structure, and the logging
 * subsystem will pick out the elements within the hse_err_t structure and
 * format them in the text log file as well as store them as structured data
 * in the data log file.
 *
 * Currently, knowledge of what format specifiers are valid and the definition
 * of the associated formatting routines is contained with logging.c. To log
 * a structure defined in the KVS component, logging.c has to include code
 * from KVS.
 *
 * There needs to be a way that clients of the HSE platform logging subsystem
 * can register conversion specifiers and their associated routines.
 */

#ifdef HSE_RELEASE_BUILD
#define HSE_LOG_SQUELCH_NS_DEFAULT (1000 * 1000)
#else
#define HSE_LOG_SQUELCH_NS_DEFAULT (0)
#endif

#define log_pri(_pri, _fmt, _async, _argv, ...)                         \
    do {                                                                \
        static struct event_counter hse_ev_log _dt_section = {          \
            .ev_odometer = 0,                                           \
            .ev_pri = (_pri),                                           \
            .ev_flags = EV_FLAGS_HSE_LOG,                               \
            .ev_file = __FILE__,                                        \
            .ev_line = __LINE__,                                        \
            .ev_dte = {                                                 \
                .dte_data = &hse_ev_log,                                \
                .dte_ops = &event_counter_ops,                          \
                .dte_type = DT_TYPE_ERROR_COUNTER,                      \
                .dte_line = __LINE__,                                   \
                .dte_file = __FILE__,                                   \
                .dte_func = __func__,                                   \
            }                                                           \
        };                                                              \
                                                                        \
        hse_log(&hse_ev_log, (_fmt), (_async), (_argv), ##__VA_ARGS__); \
    } while (0)


#define log_info_sync(_fmt, ...) log_pri(HSE_LOGPRI_INFO, (_fmt), false, NULL, ##__VA_ARGS__)

#define log_debug(_fmt, ...)    log_pri(HSE_LOGPRI_DEBUG, (_fmt), true, NULL, ##__VA_ARGS__)
#define log_info(_fmt, ...)     log_pri(HSE_LOGPRI_INFO, (_fmt), true, NULL, ##__VA_ARGS__)
#define log_warn(_fmt, ...)     log_pri(HSE_LOGPRI_WARN, (_fmt), true, NULL, ##__VA_ARGS__)
#define log_err(_fmt, ...)      log_pri(HSE_LOGPRI_ERR, (_fmt), false, NULL, ##__VA_ARGS__)
#define log_crit(_fmt, ...)     log_pri(HSE_LOGPRI_CRIT, (_fmt), false, NULL, ##__VA_ARGS__)

#define log_prix(_pri, _fmt, _async, _err, ...)                         \
    do {                                                                \
        void *av[] = { &(_err), NULL };                                 \
                                                                        \
        log_pri((_pri), (_fmt), (_async), av, ##__VA_ARGS__);           \
    } while (0)

#define log_warnx(_fmt, _err, ...)  log_prix(HSE_LOGPRI_WARN, (_fmt), true, (_err), ##__VA_ARGS__)
#define log_errx(_fmt, _err, ...)   log_prix(HSE_LOGPRI_ERR, (_fmt), false, (_err), ##__VA_ARGS__)


void
hse_log(struct event_counter *ev, const char *fmt, bool async, void **args, ...) HSE_PRINTF(2, 5);

const char *
hse_logpri_val_to_name(hse_logpri_t val);

hse_logpri_t
hse_logpri_name_to_val(const char *name);

struct hse_log_fmt_state;

typedef bool
hse_log_fmt_func_t(char **pos, char *end, void *obj);

typedef bool
hse_log_add_func_t(struct hse_log_fmt_state *state, void *obj);

bool
hse_log_register(int code, hse_log_fmt_func_t *fmt, hse_log_add_func_t *add);

bool
hse_log_deregister(int code);

bool
hse_log_push(struct hse_log_fmt_state *state, bool indexed, const char *name, const char *value);

struct slog;

enum slog_token {
    _SLOG_START_TOKEN = 1,
    _SLOG_CHILD_START_TOKEN,
    _SLOG_FIELD_TOKEN,
    _SLOG_LIST_TOKEN,
    _SLOG_CHILD_END_TOKEN,
    _SLOG_END_TOKEN
};

#define HSE_SLOG_START(type)        NULL, _SLOG_START_TOKEN, "type", "%s", (type)
#define HSE_SLOG_CHILD_START(key)   _SLOG_CHILD_START_TOKEN, (key)
#define HSE_SLOG_CHILD_END          _SLOG_CHILD_END_TOKEN
#define HSE_SLOG_END                _SLOG_END_TOKEN

#define HSE_SLOG_FIELD(key, fmt, val) \
    hse_slog_validate_field(fmt, val), (key), (fmt), (val)

#define HSE_SLOG_LIST(key, fmt, cnt, val) \
    hse_slog_validate_list(fmt, val[0]), (key), (fmt), (cnt), (val)

#define hse_slog_append(_logger, ...) \
    hse_slog_append_internal((_logger), __VA_ARGS__, NULL)

#define slog_debug(...)     hse_slog_internal(HSE_LOGPRI_DEBUG, __VA_ARGS__, NULL)
#define slog_info(...)      hse_slog_internal(HSE_LOGPRI_INFO, __VA_ARGS__, NULL)
#define slog_warn(...)      hse_slog_internal(HSE_LOGPRI_WARN, __VA_ARGS__, NULL)
#define slog_err(...)       hse_slog_internal(HSE_LOGPRI_ERR, __VA_ARGS__, NULL)

void
hse_slog_internal(hse_logpri_t priority, const char *fmt, ...);

int
hse_slog_create(hse_logpri_t priority, struct slog **sl, const char *type);

int
hse_slog_append_internal(struct slog *sl, ...);

int
hse_slog_commit(struct slog *sl);

static inline HSE_PRINTF(1, 2) int hse_slog_validate_field(char *fmt, ...)
{
    return _SLOG_FIELD_TOKEN;
}

static inline HSE_PRINTF(1, 2) int hse_slog_validate_list(char *fmt, ...)
{
    return _SLOG_LIST_TOKEN;
}

extern FILE *hse_log_file;

/* clang-format on */

#endif /* HSE_PLATFORM_LOGGING_H */
