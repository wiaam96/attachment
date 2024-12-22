#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "nano_attachment.h"
#include "nano_initializer.h"
#include "nano_attachment_common.h"
#include "attachment_types.h"

#define NUM_OF_ITERATIONS 10000

nano_str_t
create_nano_str(const char *str)
{
    nano_str_t nano_str;
    nano_str.data = (unsigned char *)(str);
    nano_str.len = strlen(str);
    return nano_str;
}

void
print_timestamp(struct timespec *ts)
{
    char buffer[30];
    struct tm *tm_info;

    tm_info = localtime(&ts->tv_sec);
    strftime(buffer, 30, "%H:%M:%S", tm_info);
    printf("%s.%09ld\n", buffer, ts->tv_nsec);
}

void
print_finalized_timestamp(struct timespec *start, struct timespec *end, const char *function_name)
{
    printf("\nFunction: %s\n", function_name);
    printf("Start: ");
    print_timestamp(start);
    printf("  End: ");
    print_timestamp(end);
    long elapsed_ns = (end->tv_sec - start->tv_sec) * 1e9 + (end->tv_nsec - start->tv_nsec);
    printf("Elapsed time: %ld ns\n", elapsed_ns);
}

int main() {
    struct timespec start, end;
    struct timespec start_time, end_time;
    int accept_verdict_cnt = 0;
    int drop_verdict_cnt = 0;
    SessionID session_id;
    long elapsed_ns_total = 0;

    NanoAttachment *attachment = InitNanoAttachment(
        NGINX_ATT_ID,
        0,
        1,
        fileno(stdout));
    while (attachment == NULL) {
        printf("Failed to Init Nano Attachment");
        return 1;
        // sleep(3);
        // attachment = InitNanoAttachment(
        //     NGINX_ATT_ID,
        //     0,
        //     1,
        //     fileno(stdout));
    }


    // AttachmentVerdictResponse inspect_response = {
    //     AttachmentVerdict::ATTACHMENT_VERDICT_INSPECT,
    //     1,
    //     NULL,
    //     NULL
    // };

    // AttachmentVerdictResponse accept_response = {
    //     AttachmentVerdict::ATTACHMENT_VERDICT_ACCEPT,
    //     1,
    //     NULL,
    //     NULL
    // };

    // AttachmentVerdictResponse custom_drop_response = {
    //     AttachmentVerdict::ATTACHMENT_VERDICT_DROP,
    //     1,
    //     NULL,
    //     NULL
    // };

    HttpMetaData http_meta_data = {
        create_nano_str("HTTP/1.1"),
        create_nano_str("GET"),
        create_nano_str("20.20.20.31"),
        create_nano_str("20.20.20.31"),
        8085,
        create_nano_str("/resetdata"),
        create_nano_str("20.20.20.31"),
        8085,
        create_nano_str("20.20.20.31"),
        create_nano_str("/resetdata")
    };

    // // Bad drop
    // HttpMetaData http_meta_data = {
    //     create_nano_str("HTTP/1.1"),
    //     create_nano_str("GET"),
    //     create_nano_str("20.20.20.31"),
    //     // create_nano_str("192.168.1.100"),
    //     create_nano_str("20.20.20.31"),
    //     8085,
    //     // create_nano_str("/dogs.html"),
    //     create_nano_str("/resetdemo?a=jndi:"),
    //     create_nano_str("20.20.20.31"),
    //     8085,
    //     create_nano_str("20.20.20.31"),
    //     create_nano_str("/resetdemo?a=jndi:")
    // };

    HttpHeaderData http_headers[13] = {
        {
            create_nano_str("Accept"),
            create_nano_str("*/*")
        },
        {
            create_nano_str("Accept-Encoding"),
            create_nano_str("gzip, deflate")
        },
        {
            create_nano_str("User-Agent"),
            create_nano_str("Mozilla/5.0 (pc-x86_64-linux-gnu) Siege/4.0.4")
        },
        {
            create_nano_str("Host"),
            create_nano_str("perftest")
        },
        {
            create_nano_str("Connection"),
            create_nano_str("close")
        }
    };

    HttpHeaders http_headers_data = {
        http_headers,
        5
    };

    // nano_str_t body[3] = {
    //     create_nano_str("Hello"),
    //     create_nano_str("World"),
    //     create_nano_str("!")
    // };

    // HttpBody http_body_data = {
    //     body,
    //     3
    // };

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (session_id = 1; session_id <= NUM_OF_ITERATIONS; session_id++) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        HttpSessionData *session_data = InitSessionData(attachment, session_id);

        AttachmentData start_data = {
            1,
            HTTP_REQUEST_METADATA,
            session_data,
            (DataBuffer)&http_meta_data
        };

        AttachmentData req_header_data = {
            1,
            HTTP_REQUEST_HEADER,
            session_data,
            (DataBuffer)&http_headers_data
        };

        // AttachmentData req_body_data = {
        //     1,
        //     HTTP_REQUEST_BODY,
        //     session_data,
        //     (DataBuffer)&http_body_data
        // };

        AttachmentData req_end_data = {
            1,
            HTTP_REQUEST_END,
            session_data,
            NULL
        };

        // clock_gettime(CLOCK_REALTIME, &start_time);
        AttachmentVerdictResponse response = SendDataNanoAttachment(attachment, &start_data);
        // AttachmentVerdictResponse response = SendDataNanoAttachmentWrapper(attachment, start_data);
        // clock_gettime(CLOCK_MONOTONIC, &end_time);
        // print_finalized_timestamp(&start_time, &end_time, "start_data");

        FreeAttachmentResponseContent(attachment, session_data, &response);

        // clock_gettime(CLOCK_MONOTONIC, &start_time);
        response = SendDataNanoAttachment(attachment, &req_header_data);
        // response = SendDataNanoAttachmentWrapper(attachment, req_header_data);
        // clock_gettime(CLOCK_MONOTONIC, &end_time);
        // print_finalized_timestamp(&start_time, &end_time, "headers_data");

        FreeAttachmentResponseContent(attachment, session_data, &response);
        // response = SendDataNanoAttachmentWrapper(attachment, &req_body_data);
        // FreeAttachmentResponseContent(attachment, session_data, &response);
        // clock_gettime(CLOCK_MONOTONIC, &start_time);
        response = SendDataNanoAttachment(attachment, &req_end_data);
        // clock_gettime(CLOCK_MONOTONIC, &end_time);
        // print_finalized_timestamp(&start_time, &end_time, "end_data");

        if (response.verdict == ATTACHMENT_VERDICT_ACCEPT) {
            accept_verdict_cnt++;
        }
        if (response.verdict == ATTACHMENT_VERDICT_DROP) {
            drop_verdict_cnt++;
        }
        FreeAttachmentResponseContent(attachment, session_data, &response);
        FiniSessionData(attachment, session_data);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        long elapsed_ns = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
        elapsed_ns_total += elapsed_ns;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n");
    printf("Time used: %f seconds\n", elapsed_time);
    printf("Accept verdict count: %d\n", accept_verdict_cnt);
    printf("Drop verdict count: %d\n", drop_verdict_cnt);
    printf("Average elapsed time: %ld ns\n: ", elapsed_ns_total/NUM_OF_ITERATIONS);

    FiniNanoAttachment(attachment);

    return 0;
}
