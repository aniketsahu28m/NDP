/* Lab 4 Q2_Unbound: Fully Recursive DNS Server (TCP)
 * Implements REAL iterative DNS resolution like Unbound
 * Starts from root servers, follows referrals through TLD and authoritative servers
 * Prints each step/layer of the resolution process
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>

#define PORT 5003
#define MAX 1024
#define DNS_PORT 53
#define MAX_RECURSION_DEPTH 10
#define MAX_NS_RECORDS 10

struct DNSHeader {
    unsigned short id;
    unsigned short flags;
    unsigned short qdcount;
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
};

struct DNSQuestion {
    unsigned short qtype;
    unsigned short qclass;
};

struct NSRecord {
    char name[256];
    char ip[20];
};

const char *ROOT_SERVERS[] = {
    "198.41.0.4",       /* a.root-servers.net */
    "199.9.14.201",     /* b.root-servers.net */
    "192.33.4.12",      /* c.root-servers.net */
    "199.7.91.13",      /* d.root-servers.net */
    "192.203.230.10",   /* e.root-servers.net */
    "192.5.5.241",      /* f.root-servers.net */
    "192.112.36.4",     /* g.root-servers.net */
    "198.97.190.53"     /* h.root-servers.net */
};
const char *ROOT_SERVER_NAMES[] = {
    "a.root-servers.net",
    "b.root-servers.net",
    "c.root-servers.net",
    "d.root-servers.net",
    "e.root-servers.net",
    "f.root-servers.net",
    "g.root-servers.net",
    "h.root-servers.net"
};
#define NUM_ROOT_SERVERS 8

static int g_step = 0;
static char g_resolution_log[4096];

void print_layer(int depth, const char *server_name, const char *server_ip)
{
    g_step++;
    const char *layer;
    
    switch(depth) {
        case 0: layer = "ROOT"; break;
        case 1: layer = "TLD"; break;
        case 2: layer = "AUTH"; break;
        default: layer = "SUB"; break;
    }
    
    printf("\n[Step %d] %s Layer\n", g_step, layer);
    printf("  Querying: %s (%s)\n", server_name, server_ip);
    
    char temp[256];
    snprintf(temp, sizeof(temp), "  [%d] %s -> %s (%s)\n", 
             g_step, layer, server_name, server_ip);
    strncat(g_resolution_log, temp, sizeof(g_resolution_log) - strlen(g_resolution_log) - 1);
}

void encode_domain_name(const char *domain, unsigned char *encoded)
{
    int i, j = 0;
    int len = strlen(domain);
    int label_start = 0;
    
    for (i = 0; i <= len; i++)
    {
        if (domain[i] == '.' || domain[i] == '\0')
        {
            encoded[j++] = i - label_start;
            while (label_start < i)
                encoded[j++] = domain[label_start++];
            label_start = i + 1;
        }
    }
    encoded[j] = 0;
}

int decode_domain_name(unsigned char *response, unsigned char *ptr, char *name, int max_len)
{
    int jumped = 0;
    int bytes_consumed = 0;
    int name_pos = 0;
    unsigned char *save_ptr = ptr;
    
    name[0] = '\0';
    
    while (*ptr != 0)
    {
        if ((*ptr & 0xC0) == 0xC0)
        {
            if (!jumped)
                bytes_consumed = (ptr - save_ptr) + 2;
            int offset = ((*ptr & 0x3F) << 8) | *(ptr + 1);
            ptr = response + offset;
            jumped = 1;
        }
        else
        {
            int label_len = *ptr++;
            if (name_pos > 0 && name_pos < max_len - 1)
                name[name_pos++] = '.';
            for (int i = 0; i < label_len && name_pos < max_len - 1; i++)
                name[name_pos++] = tolower(*ptr++);
        }
    }
    
    name[name_pos] = '\0';
    
    if (!jumped)
        bytes_consumed = (ptr - save_ptr) + 1;
    
    return bytes_consumed;
}

int build_dns_query(const char *domain, unsigned char *query, unsigned short qtype)
{
    struct DNSHeader *header = (struct DNSHeader *)query;
    
    header->id = htons((getpid() ^ time(NULL)) & 0xFFFF);
    header->flags = htons(0x0000);  /* RD=0: iterative query */
    header->qdcount = htons(1);
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;
    
    unsigned char *qname = query + 12;
    encode_domain_name(domain, qname);
    
    int qname_len = strlen((char *)qname) + 1;
    struct DNSQuestion *question = (struct DNSQuestion *)(qname + qname_len);
    question->qtype = htons(qtype);
    question->qclass = htons(1);
    
    return 12 + qname_len + 4;
}

/* Returns: 1 = answer, 0 = referral, -1 = error, -2 = NODATA */
int parse_dns_response(unsigned char *response, int resp_len,
                       char *ip_result,
                       struct NSRecord *ns_records, int *ns_count,
                       char *referral_zone)
{
    struct DNSHeader *header = (struct DNSHeader *)response;
    int ancount = ntohs(header->ancount);
    int nscount = ntohs(header->nscount);
    int arcount = ntohs(header->arcount);
    unsigned short flags = ntohs(header->flags);
    int rcode = flags & 0x000F;
    int aa = (flags >> 10) & 0x1;
    
    printf("  Response: answers=%d, authority=%d, additional=%d, rcode=%d\n", 
           ancount, nscount, arcount, rcode);
    
    referral_zone[0] = '\0';
    
    if (rcode == 3) {
        printf("  NXDOMAIN: Domain does not exist\n");
        return -1;
    }
    if (rcode != 0) {
        printf("  DNS Error: rcode=%d\n", rcode);
        return -1;
    }
    
    *ns_count = 0;
    unsigned char *ptr = response + 12;
    
    /* Skip question */
    while (*ptr != 0) ptr++;
    ptr += 5;
    
    /* Parse answers */
    printf("  ANSWERS:\n");
    for (int i = 0; i < ancount; i++)
    {
        char name[256];
        int name_bytes = decode_domain_name(response, ptr, name, sizeof(name));
        ptr += name_bytes;
        
        unsigned short type = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        ptr += 2;
        unsigned int ttl = ntohl(*(unsigned int *)ptr);
        ptr += 4;
        unsigned short rdlength = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        
        if (type == 1 && rdlength == 4)
        {
            sprintf(ip_result, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
            printf("    A: %s -> %s (TTL=%u)\n", name, ip_result, ttl);
            return 1;
        }
        else if (type == 5)
        {
            char cname[256];
            decode_domain_name(response, ptr, cname, sizeof(cname));
            printf("    CNAME: %s -> %s\n", name, cname);
        }
        ptr += rdlength;
    }
    if (ancount == 0)
        printf("    (none)\n");
    
    /* Parse authority (NS referrals) */
    printf("  AUTHORITY (referrals):\n");
    int found_soa = 0;
    for (int i = 0; i < nscount && *ns_count < MAX_NS_RECORDS; i++)
    {
        char name[256];
        int name_bytes = decode_domain_name(response, ptr, name, sizeof(name));
        ptr += name_bytes;
        
        unsigned short type = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        ptr += 2;
        ptr += 4;
        unsigned short rdlength = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        
        if (type == 2)
        {
            char ns_name[256];
            decode_domain_name(response, ptr, ns_name, sizeof(ns_name));
            strncpy(ns_records[*ns_count].name, ns_name, 255);
            ns_records[*ns_count].ip[0] = '\0';
            printf("    NS: %s -> %s\n", name, ns_name);
            if (referral_zone[0] == '\0')
                strncpy(referral_zone, name, 255);
            (*ns_count)++;
        }
        else if (type == 6)
        {
            printf("    SOA: authoritative negative\n");
            found_soa = 1;
        }
        ptr += rdlength;
    }
    if (nscount == 0)
        printf("    (none)\n");
    
    if (found_soa && ancount == 0 && aa)
        return -2;
    
    /* Parse additional (glue records) */
    printf("  ADDITIONAL (glue):\n");
    int glue_found = 0;
    for (int i = 0; i < arcount; i++)
    {
        char name[256];
        int name_bytes = decode_domain_name(response, ptr, name, sizeof(name));
        ptr += name_bytes;
        
        unsigned short type = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        ptr += 2;
        ptr += 4;
        unsigned short rdlength = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        
        if (type == 1 && rdlength == 4)
        {
            char ip[20];
            sprintf(ip, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
            printf("    GLUE: %s -> %s\n", name, ip);
            glue_found++;
            
            for (int j = 0; j < *ns_count; j++)
            {
                if (strcasecmp(ns_records[j].name, name) == 0)
                    strncpy(ns_records[j].ip, ip, 19);
            }
        }
        ptr += rdlength;
    }
    if (!glue_found)
        printf("    (none)\n");
    
    if (*ns_count > 0)
        return 0;
    
    return -1;
}

int query_dns_server(const char *dns_server, const char *domain,
                     char *ip_result,
                     struct NSRecord *ns_records, int *ns_count,
                     char *referral_zone)
{
    int sockfd;
    struct sockaddr_in dnsaddr;
    unsigned char query[MAX], response[MAX];
    int query_len, resp_len;
    socklen_t addrlen;
    struct timeval timeout;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) return -1;
    
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    memset(&dnsaddr, 0, sizeof(dnsaddr));
    dnsaddr.sin_family = AF_INET;
    dnsaddr.sin_port = htons(DNS_PORT);
    dnsaddr.sin_addr.s_addr = inet_addr(dns_server);
    
    memset(query, 0, MAX);
    query_len = build_dns_query(domain, query, 1);
    
    printf("  Sending query for: %s\n", domain);
    
    if (sendto(sockfd, query, query_len, 0, 
               (struct sockaddr *)&dnsaddr, sizeof(dnsaddr)) == -1)
    {
        printf("  ERROR: Failed to send\n");
        close(sockfd);
        return -1;
    }
    
    memset(response, 0, MAX);
    addrlen = sizeof(dnsaddr);
    resp_len = recvfrom(sockfd, response, MAX, 0,
                        (struct sockaddr *)&dnsaddr, &addrlen);
    close(sockfd);
    
    if (resp_len <= 0)
    {
        printf("  ERROR: Timeout\n");
        return -1;
    }
    
    printf("  Received %d bytes\n", resp_len);
    return parse_dns_response(response, resp_len, ip_result, ns_records, ns_count, referral_zone);
}

int resolve_ns_to_ip(const char *ns_name, char *ip_result, int depth);

int resolve_iteratively(const char *domain, char *ip_result, int depth)
{
    if (depth >= MAX_RECURSION_DEPTH)
    {
        printf("  ERROR: Max recursion depth\n");
        return -1;
    }
    
    struct NSRecord ns_records[MAX_NS_RECORDS];
    int ns_count = 0;
    int result;
    char referral_zone[256] = {0};
    char last_zone[256] = {0};
    
    print_layer(0, ROOT_SERVER_NAMES[0], ROOT_SERVERS[0]);
    
    result = query_dns_server(ROOT_SERVERS[0], domain, 
                              ip_result, ns_records, &ns_count, referral_zone);
    
    if (result == 1)
    {
        printf("  => RESOLVED: %s\n", ip_result);
        return 0;
    }
    
    if (result < 0)
        return -1;
    
    strncpy(last_zone, referral_zone, 255);
    
    int current_depth = 1;
    while (result == 0 && ns_count > 0 && current_depth < MAX_RECURSION_DEPTH)
    {
        int resolved_next = 0;
        
        for (int i = 0; i < ns_count && !resolved_next; i++)
        {
            char ns_ip[20];
            
            if (ns_records[i].ip[0] != '\0')
            {
                strncpy(ns_ip, ns_records[i].ip, 19);
            }
            else
            {
                printf("\n  [Side query: resolving NS %s]\n", ns_records[i].name);
                if (resolve_ns_to_ip(ns_records[i].name, ns_ip, depth + 1) != 0)
                {
                    printf("  Could not resolve NS, trying next...\n");
                    continue;
                }
                printf("  NS resolved to: %s\n", ns_ip);
            }
            
            print_layer(current_depth, ns_records[i].name, ns_ip);
            
            struct NSRecord new_ns[MAX_NS_RECORDS];
            int new_ns_count = 0;
            char new_zone[256] = {0};
            
            result = query_dns_server(ns_ip, domain,
                                      ip_result, new_ns, &new_ns_count, new_zone);
            
            if (result == 1)
            {
                printf("  => RESOLVED: %s\n", ip_result);
                return 0;
            }
            else if (result == -2)
            {
                printf("  => No A record exists\n");
                return -1;
            }
            else if (result == 0 && new_ns_count > 0)
            {
                if (new_zone[0] != '\0' && strcasecmp(new_zone, last_zone) == 0)
                {
                    printf("  WARNING: Loop detected (same zone: %s)\n", new_zone);
                    return -1;
                }
                
                memcpy(ns_records, new_ns, sizeof(ns_records));
                ns_count = new_ns_count;
                strncpy(last_zone, new_zone, 255);
                resolved_next = 1;
                current_depth++;
            }
        }
        
        if (!resolved_next)
        {
            printf("  ERROR: No valid nameservers\n");
            return -1;
        }
    }
    
    return -1;
}

int resolve_ns_to_ip(const char *ns_name, char *ip_result, int depth)
{
    if (depth >= MAX_RECURSION_DEPTH)
        return -1;
    
    struct sockaddr_in dnsaddr;
    unsigned char query[MAX], response[MAX];
    int sockfd;
    struct timeval timeout;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) return -1;
    
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    memset(&dnsaddr, 0, sizeof(dnsaddr));
    dnsaddr.sin_family = AF_INET;
    dnsaddr.sin_port = htons(DNS_PORT);
    dnsaddr.sin_addr.s_addr = inet_addr(ROOT_SERVERS[1]);
    
    memset(query, 0, MAX);
    int query_len = build_dns_query(ns_name, query, 1);
    
    struct DNSHeader *hdr = (struct DNSHeader *)query;
    hdr->flags = htons(0x0100);  /* RD=1 for helper query */
    
    if (sendto(sockfd, query, query_len, 0,
               (struct sockaddr *)&dnsaddr, sizeof(dnsaddr)) == -1)
    {
        close(sockfd);
        return -1;
    }
    
    memset(response, 0, MAX);
    socklen_t addrlen = sizeof(dnsaddr);
    int resp_len = recvfrom(sockfd, response, MAX, 0,
                            (struct sockaddr *)&dnsaddr, &addrlen);
    close(sockfd);
    
    if (resp_len <= 0) return -1;
    
    struct DNSHeader *header = (struct DNSHeader *)response;
    int ancount = ntohs(header->ancount);
    
    if (ancount == 0) return -1;
    
    unsigned char *ptr = response + 12;
    while (*ptr != 0) ptr++;
    ptr += 5;
    
    for (int i = 0; i < ancount; i++)
    {
        if ((*ptr & 0xC0) == 0xC0) ptr += 2;
        else { while (*ptr != 0) ptr++; ptr++; }
        
        unsigned short type = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        ptr += 2;
        ptr += 4;
        unsigned short rdlength = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        
        if (type == 1 && rdlength == 4)
        {
            sprintf(ip_result, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
            return 0;
        }
        ptr += rdlength;
    }
    
    return -1;
}

int resolve_domain(const char *domain, char *result, char *detailed_log)
{
    char ip_result[20] = {0};
    
    g_step = 0;
    g_resolution_log[0] = '\0';
    
    printf("\n=== Resolving: %s ===\n", domain);
    printf("Starting from root servers (iterative, RD=0)\n");
    
    strncat(g_resolution_log, "Resolution path:\n", sizeof(g_resolution_log) - 1);
    
    int ret = resolve_iteratively(domain, ip_result, 0);
    
    if (ret == 0)
    {
        printf("\nFINAL: %s -> %s (%d steps)\n\n", domain, ip_result, g_step);
        snprintf(result, MAX, "%s", ip_result);
        snprintf(detailed_log, 2048, "%s\nFinal: %s -> %s (%d steps)", 
                 g_resolution_log, domain, ip_result, g_step);
        return 0;
    }
    else
    {
        printf("\nFAILED: %s\n\n", domain);
        snprintf(detailed_log, 2048, "%s\nResolution FAILED", g_resolution_log);
        return -1;
    }
}

int main(void)
{
    int sockfd, newsockfd, retval;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char domain[MAX], response[MAX], ip_result[20], detailed_log[2048];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed\n");
        exit(0);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&servaddr, '\0', sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    retval = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (retval == -1)
    {
        printf("Binding failed\n");
        close(sockfd);
        exit(0);
    }

    retval = listen(sockfd, 5);
    if (retval == -1)
    {
        printf("Listen failed\n");
        close(sockfd);
        exit(0);
    }

    printf("Recursive DNS Server (Unbound-style)\n");
    printf("Listening on port %d\n\n", PORT);

    len = sizeof(cliaddr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
    if (newsockfd == -1)
    {
        printf("Accept failed\n");
        close(sockfd);
        exit(0);
    }

    printf("Client connected: %s:%d\n\n", 
           inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    while (1)
    {
        memset(domain, 0, MAX);
        memset(response, 0, MAX);
        memset(ip_result, 0, sizeof(ip_result));
        memset(detailed_log, 0, sizeof(detailed_log));

        if (recv(newsockfd, domain, MAX, 0) <= 0)
            break;

        int dlen = strlen(domain);
        while (dlen > 0 && (domain[dlen-1] == '\n' || domain[dlen-1] == '\r' || domain[dlen-1] == ' '))
            domain[--dlen] = '\0';

        if (strcmp(domain, "exit") == 0)
            break;

        if (resolve_domain(domain, ip_result, detailed_log) == 0)
        {
            snprintf(response, MAX, "Resolved:\n%s", detailed_log);
        }
        else
        {
            snprintf(response, MAX, "Failed:\n%s", detailed_log);
        }

        send(newsockfd, response, strlen(response), 0);
    }

    printf("Client disconnected.\n");
    close(newsockfd);
    close(sockfd);
    return 0;
}
