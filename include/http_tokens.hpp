/* ************************************************************************ */
/*                                                                          */
/*                          Class: Http traits                              */
/*                                                                          */
/* ************************************************************************ */

#ifndef __TRAITS_HPP__
# define __TRAITS_HPP__

#include <string>
#include <map>

namespace ws {
namespace http {

//   HTTP-message   = start-line CRLF
//                    *( field-line CRLF )
//                    CRLF
//                    [ message-body ]
//  start-line     = request-line / status-line

// ANY other version request is rejected. This is also response version.
#define WS_HTTP_VERSION "HTTP/1.1"

// ascii tokens
#define LF_int 10
#define CR_int 13
#define SP_int 32

#define CRLF "\r\n"
#define SP " "

// Common HTTP status codes --------------------------------------------------
// INFORMATION RESPONSES
#define WS_100_CONTINUE 100
#define WS_101_SWITCHING_PROTOCOLS 101
#define WS_103_EARLY_HINTS 103
// SUCCESS RESPONSES
#define WS_200_OK 200
#define WS_201_CREATED 201
#define WS_202_ACCEPTED 202
#define WS_203_NON_AUTORATIVE_INFORMATION 203
#define WS_204_NO_CONTENT 204
#define WS_205_RESET_CONTENT 205
#define WS_206_PARTIAL_CONTENT 206
// REDIRECTION MESSAGES
#define WS_300_MULTIPLE_CHOICES 300
#define WS_301_MOVED_PERMANENTLY 301
#define WS_302_FOUND 302
#define WS_303_SEE_OTHER 303
#define WS_304_NOT_MODIFIED 304
#define WS_307_TEMPORARY_REDIRECT 307
#define WS_308_PERMANENT_REDIRECT 308
// CLIENT ERROR RESPONSES
#define WS_400_BAD_REQUEST 400
#define WS_401_UNAUTHORIZED 401
#define WS_402_PAYMENT_REQUIRED 402
#define WS_403_FORBIDDEN 403
#define WS_404_NOT_FOUND 404
#define WS_405_METHOD_NOT_ALLOWED 405
#define WS_406_NOT_ACCEPTABLE 406
#define WS_407_PROXY_AUTHENTICATION_REQUIRED 407
#define WS_408_REQUEST_TIMEOUT 408
#define WS_409_CONFLICT 409
#define WS_410_GONE 410
#define WS_411_LENGTH_REQUIRED 411
#define WS_412_PRECONDITION_FAILED 412
#define WS_413_PAYLOAD_TOO_LARGE 413
#define WS_414_URI_TOO_LONG 414
#define WS_415_UNSUPPORTED_MEDIA_TYPE 415
#define WS_416_RANGE_NOT_SATISFYABLE 416
#define WS_417_EXPECTATION_FAILED 417
#define WS_418_IMATEAPOT 418
#define WS_416_UPGRADE_REQUIRED 426
#define WS_428_PRECONDITION_REQUIRED 428
#define WS_429_TOO_MANY_REQUESTS 429
#define WS_431_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define WS_451_UNAVAILABLE_FOR_LEGAL_REASONS 451
// SERVER ERROR RESPONSES
#define WS_500_INTERNAL_SERVER_ERROR 500
#define WS_501_NOT_IMPLEMENTED 501
#define WS_502_BAD_GATEWAY 502
#define WS_503_SERVICE_UNAVAILABLE 503
#define WS_504_GATEWAY_TIMEOUT 504
#define WS_506_VARIANT_ALSO_NEGOTIATES 506
#define WS_507_HTTP_VERSION_NOT_SUPPORTED 507
#define WS_511_NETWORK_AUTHENTICATION_REQUIRED 511
// ---------------------------------------------------------------------------
#define BUFFER_SIZE 100000

// It is RECOMMENDED that all HTTP senders and recipients support, at a 
// minimum, request-line lengths of 8000 octets.
#define REQUEST_LINE_LENGTH 9000
#define MAX_WORD_LENGTH 9000

// HTTP does not place a predefined limit on the length of a request-line, as described in Section 2.3 of [HTTP]. 
// A server that receives a method longer than any that it implements SHOULD respond with a 501 (Not Implemented) 
// status code. A server that receives a request-target longer than any URI it wishes to parse MUST respond with
// a 414 (URI Too Long) status code (see Section 15.5.15 of [HTTP]).
#define TARGET_SIZE 1024

#define MAX_FIELD_LENGTH 8192 // 8KB like Apache

class StatusPhrase {
    public:
        StatusPhrase() { _map_codes(); }
        const char* operator[](int n) const { return (status_code[n]); }
    private:
        const char *status_code[512];
        void _map_codes() {
            bzero(status_code, 512);
            status_code[100] = "100 (CONTINUE)";
            status_code[101] = "101 (SWITCHING PROTOCOLS)";
            status_code[103] = "103 (EARLY HINTS)";
            status_code[200] = "200 (OK)";
            status_code[201] = "201 (CREATED)";
            status_code[202] = "202 (ACCEPTED)";
            status_code[203] = "203 (NON AUTORATIVE INFORMATION)";
            status_code[204] = "204 (NO CONTENT)";
            status_code[205] = "205 (RESET CONTENT)";
            status_code[206] = "206 (PARTIAL CONTENT)";
            status_code[300] = "300 (MULTIPLE CHOICES)";
            status_code[301] = "301 (MOVED PERMANENTLY)";
            status_code[302] = "302 (FOUND)";
            status_code[303] = "303 (SEE OTHER)";
            status_code[304] = "304 (NOT MODIFIED)";
            status_code[307] = "307 (TEMPORARY REDIRECT)";
            status_code[308] = "308 (PERMANENT REDIRECT)";
            status_code[400] = "400 (BAD REQUEST)";
            status_code[401] = "401 (UNAUTHORIZED)";
            status_code[402] = "402 (PAYMENT REQUIRED)";
            status_code[403] = "403 (FORBIDDEN)";
            status_code[404] = "404 (NOT FOUND)";
            status_code[405] = "405 (METHOD NOT ALLOWED)";
            status_code[406] = "406 (NOT ACCEPTABLE)";
            status_code[407] = "407 (PROXY AUTHENTICATION REQUIRED)";
            status_code[408] = "408 (REQUEST TIMEOUT)";
            status_code[409] = "409 (CONFLICT)";
            status_code[410] = "410 (GONE)";
            status_code[411] = "411 (LENGTH REQUIRED)";
            status_code[412] = "412 (PRECONDITION FAILED)";
            status_code[413] = "413 (PAYLOAD TOO LARGE)";
            status_code[414] = "414 (URI TOO LONG)";
            status_code[415] = "415 (UNSUPPORTED MEDIA TYPE)";
            status_code[416] = "416 (RANGE NOT SATISFYABLE)";
            status_code[417] = "417 (EXPECTATION FAILED)";
            status_code[418] = "418 (I'M A TEAPOT)";
            status_code[426] = "416 (UPGRADE REQUIRED)";
            status_code[428] = "428 (PRECONDITION REQUIRED)";
            status_code[429] = "429 (TOO MANY REQUESTS)";
            status_code[431] = "431 (REQUEST HEADER FIELDS TOO LARGE)";
            status_code[451] = "451 (UNAVAILABLE FOR LEGAL REASONS)";
            status_code[500] = "500 (INTERNAL SERVER ERROR)";
            status_code[501] = "501 (NOT IMPLEMENTED)";
            status_code[502] = "502 (BAD GATEWAY)";
            status_code[503] = "503 (SERVICE UNAVAILABLE)";
            status_code[504] = "504 (GATEWAY TIMEOUT)";
            status_code[506] = "506 (VARIANT ALSO NEGOTIATES)";
            status_code[507] = "507 (HTTP VERSION NOT SUPPORTED)";
            status_code[511] = "511 (NETWORK AUTHENTICATION REQUIRE)";
        }
}; // CLASS StatusPhrase

// mapps extensions to type field eg ".ico" to "image/x-icon"
class ExtensionsToType {
public :
    std::map<std::string, std::string> typemap;
    ExtensionsToType(const ExtensionsToType& other): typemap(other.typemap) {}
    ExtensionsToType& operator=(const ExtensionsToType& other) { typemap = other.typemap; return(*this); }
    ExtensionsToType() {
        typemap.insert(std::make_pair<std::string, std::string>(  "aac",    "audio/aac"));
        typemap.insert(std::make_pair<std::string, std::string>(  "abw",    "application/x-abiword"));
        typemap.insert(std::make_pair<std::string, std::string>(  "arc",    "application/x-freearc"));
        typemap.insert(std::make_pair<std::string, std::string>(  "avif",   "image/avif"));
        typemap.insert(std::make_pair<std::string, std::string>(  "avi",    "video/x-msvideo"));
        typemap.insert(std::make_pair<std::string, std::string>(  "azw",    "application/vnd.amazon.ebook"));
        typemap.insert(std::make_pair<std::string, std::string>(  "bin",    "application/octet-stream"));
        typemap.insert(std::make_pair<std::string, std::string>(  "bmp",    "image/bmp"));
        typemap.insert(std::make_pair<std::string, std::string>(  "bz",     "application/x-bzip"));
        typemap.insert(std::make_pair<std::string, std::string>(  "bz2",    "application/x-bzip2"));
        typemap.insert(std::make_pair<std::string, std::string>(  "cda",    "application/x-cdf"));
        typemap.insert(std::make_pair<std::string, std::string>(  "csh",    "application/x-csh"));
        typemap.insert(std::make_pair<std::string, std::string>(  "css",    "text/css"));
        typemap.insert(std::make_pair<std::string, std::string>(  "csv",    "text/csv"));
        typemap.insert(std::make_pair<std::string, std::string>(  "doc",    "application/msword"));
        typemap.insert(std::make_pair<std::string, std::string>(  "docx",   "application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
        typemap.insert(std::make_pair<std::string, std::string>(  "eot",    "application/vnd.ms-fontobject"));
        typemap.insert(std::make_pair<std::string, std::string>(  "epub",   "application/epub+zip"));
        typemap.insert(std::make_pair<std::string, std::string>(  "gz",     "application/gzip"));
        typemap.insert(std::make_pair<std::string, std::string>(  "gif",    "image/gif"));
        typemap.insert(std::make_pair<std::string, std::string>(  "html",   "text/html"));
        typemap.insert(std::make_pair<std::string, std::string>(  "htm",    "text/html"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ico",    "image/x-icon"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ics",    "text/calendar"));
        typemap.insert(std::make_pair<std::string, std::string>(  "jar",    "application/java-archive"));
        typemap.insert(std::make_pair<std::string, std::string>(  "jpg",    "image/jpeg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "jpeg",   "image/jpeg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "js",     "text/javascript"));
        typemap.insert(std::make_pair<std::string, std::string>(  "json",   "application/json"));
        typemap.insert(std::make_pair<std::string, std::string>(  "jsonld", "application/ld+json"));
        typemap.insert(std::make_pair<std::string, std::string>(  "mid",    "audio/midi"));
        typemap.insert(std::make_pair<std::string, std::string>(  "midi",   "audio/midi"));
        typemap.insert(std::make_pair<std::string, std::string>(  "mjs",    "text/javascript"));
        typemap.insert(std::make_pair<std::string, std::string>(  "mp3",    "audio/mpeg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "mp4",    "video/mp4"));
        typemap.insert(std::make_pair<std::string, std::string>(  "mpeg",   "video/mpeg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "mpkg",   "application/vnd.apple.installer+xml"));
        typemap.insert(std::make_pair<std::string, std::string>(  "odp",    "application/vnd.oasis.opendocument.presentation"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ods",    "application/vnd.oasis.opendocument.spreadsheet"));
        typemap.insert(std::make_pair<std::string, std::string>(  "odt",    "application/vnd.oasis.opendocument.text"));
        typemap.insert(std::make_pair<std::string, std::string>(  "oga",    "audio/ogg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ogv",    "video/ogg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ogx",    "application/ogg"));
        typemap.insert(std::make_pair<std::string, std::string>(  "opus",   "audio/opus"));
        typemap.insert(std::make_pair<std::string, std::string>(  "otf",    "font/otf"));
        typemap.insert(std::make_pair<std::string, std::string>(  "png",    "image/png"));
        typemap.insert(std::make_pair<std::string, std::string>(  "pdf",    "application/pdf"));
        typemap.insert(std::make_pair<std::string, std::string>(  "php",    "text/event-stream"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ppt",    "application/vnd.ms-powerpoint"));
        typemap.insert(std::make_pair<std::string, std::string>(  "pptx",   "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
        typemap.insert(std::make_pair<std::string, std::string>(  "rar",    "application/vnd.rar"));
        typemap.insert(std::make_pair<std::string, std::string>(  "rtf",    "application/rtf"));
        typemap.insert(std::make_pair<std::string, std::string>(  "sh",     "application/x-sh"));
        typemap.insert(std::make_pair<std::string, std::string>(  "svg",    "image/svg+xml"));
        typemap.insert(std::make_pair<std::string, std::string>(  "tar",    "application/x-tar"));
        typemap.insert(std::make_pair<std::string, std::string>(  "tiff",   "image/tiff"));
        typemap.insert(std::make_pair<std::string, std::string>(  "tif",    "image/tiff"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ts",     "video/mp2t"));
        typemap.insert(std::make_pair<std::string, std::string>(  "ttf",    "font/ttf"));
        typemap.insert(std::make_pair<std::string, std::string>(  "txt",    "text/plain"));
        typemap.insert(std::make_pair<std::string, std::string>(  "vsd",    "application/vnd.visio"));
        typemap.insert(std::make_pair<std::string, std::string>(  "wav",    "audio/wav"));
        typemap.insert(std::make_pair<std::string, std::string>(  "weba",   "audio/webm"));
        typemap.insert(std::make_pair<std::string, std::string>(  "webm",   "video/webm"));
        typemap.insert(std::make_pair<std::string, std::string>(  "webp",   "image/webp"));
        typemap.insert(std::make_pair<std::string, std::string>(  "woff",   "font/woff"));
        typemap.insert(std::make_pair<std::string, std::string>(  "woff2",  "font/woff2"));
        typemap.insert(std::make_pair<std::string, std::string>(  "xhtml",  "application/xhtml+xml"));
        typemap.insert(std::make_pair<std::string, std::string>(  "xls",    "application/vnd.ms-excel"));
        typemap.insert(std::make_pair<std::string, std::string>(  "xlsx",   "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
        typemap.insert(std::make_pair<std::string, std::string>(  "xml",    "application/xml"));
        typemap.insert(std::make_pair<std::string, std::string>(  "xul",    "application/vnd.mozilla.xul+xml"));
        typemap.insert(std::make_pair<std::string, std::string>(  "zip",    "application/zip"));
        typemap.insert(std::make_pair<std::string, std::string>(  "3gp",    "video/3gpp"));
        typemap.insert(std::make_pair<std::string, std::string>(  "3g2",    "video/3gpp2"));
        typemap.insert(std::make_pair<std::string, std::string>(  "7z",     "application/x-7z-compressed"));
    } // constructor ExtensionsToType();
    ~ExtensionsToType() {}

};

class Tokens {
    public:
        StatusPhrase status_phrases;
        ExtensionsToType extensions;
};

} // NAMESPACE http
} // NAMESPACE ws

#endif // __TRAITS_HPP__
