/*
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2022 Aggelos Tselios
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "../../nvdialog_assert.h"
#include "nvdialog_adw.h"
#include "nvdialog_notification.h"
#include <libnotify/notification.h>
#include <dlfcn.h>
#include <stdlib.h>

struct _NvdNotification {
        char *title;
        char *contents;
        NvdNotifyType type;
        bool shown;
        void *raw;
        void *lib;
};

typedef NotifyNotification *(*nvd_notify_new_t)(const char*, const char*, const char*);

NvdNotification *nvd_notification_adw(const char   *title,
                                      const char   *msg,
                                      NvdNotifyType type) {
        NvdNotification *notification = malloc(sizeof(struct _NvdNotification));
        NVD_RETURN_IF_NULL(notification);

        notification->lib            = dlopen("/usr/lib/libnotify.so", RTLD_LAZY);
        nvd_notify_new_t notify_new  = dlsym(notification->lib, "notify_notification_new");
        
        if (!fn) {
                nvd_error_message("libnotify is missing required symbols.");
                
                dlclose(notification->lib);
                free   (notification);

                return NULL;
        }

        notification->title    = (char*) title;
        notification->contents = (char*) msg;
        notification->raw      = (void*) notify_new(notification->title,
                                                    notification->contents,
                                                    NULL);
        notification->type     = type;
        notification->shown    = false;
        
        NVD_ASSERT(notification->lib != NULL);
        return notification;
}

void nvd_send_notification_adw(NvdNotification *notification) {
        NVD_ASSERT(notification        != NULL );
        NVD_ASSERT(notification->shown == false); /* Just to avoid halting the thread. */

        bool (*show_fn)(NotifyNotification*, GError**) = dlsym(notification->lib,
                                                              "notify_notification_show");
        if (!show_fn) {
                nvd_error_message("libnotify is missing required symbols, see assertion message below.");
                NVD_ASSERT(show_fn != NULL);
        }
        
        show_fn(notification->raw, NULL);
}

void nvd_delete_notification_adw(NvdNotification *notification) {
        NVD_ASSERT_FATAL(notification != NULL); /* Fatal to avoid some pretty scary bugs. */
        dlclose(notification->lib);
        free(notification);
}