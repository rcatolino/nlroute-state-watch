#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main() {
  struct sockaddr_nl addr;
  int ret;
  int nl_sock;
  char buff[4096];
  int length;
  struct iovec msg_iov = {buff, sizeof(buff)}; // See bits/uio.h
  struct msghdr hdr = {&addr, sizeof(addr),
                       &msg_iov, sizeof(msg_iov),
                       NULL, 0, // No ancillary data
                       0}; // No flags
                      // See bits/socket.h
  struct nlmsghdr * nlhdr; // See linux/netlink.h


  nl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (!nl_sock) {
    perror("socket ");
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  addr.nl_groups = /*RTMGRP_LINK | */RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
  // see linux/rtnetlink.h
  ret = bind(nl_sock, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1) {
    perror("Bind failed ");
    return -1;
  }

  while (1) {

    printf("Netlink, waiting for next message\n");
    // The following loop receive a potentially multipart message
    // See man 3 netlink for the macros
    length = recvmsg(nl_sock, &hdr, sizeof(hdr));
    printf("Message received!\n");
    for (nlhdr = (struct nlmsghdr *)buff; NLMSG_OK(nlhdr, length); nlhdr = NLMSG_NEXT(nlhdr, length)) {
      printf("Parsing message\n");
      if (nlhdr->nlmsg_type == NLMSG_DONE) {
        printf("Netlink, end of multipart message\n");
        break;
      }

      if (nlhdr->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr * err;
        // parse the nlmsgerr in the payload
        err = (struct nlmsgerr *) NLMSG_DATA(nlhdr);
        printf("Netlink error : %s\n", strerror(err->error));
        continue;
      }

      if (nlhdr->nlmsg_type == NLMSG_NOOP) {
        printf("Netlink noop\n");
        continue;
      }

      if (nlhdr->nlmsg_type == RTM_NEWADDR) {
        printf("RTM_NEWADDR\n");
      }

      /*
      if (nlhdr->nlmsg_type == RTM_NEWLINK) {
        printf("RTM_NEWLINK\n");
      }
      */

      if (nlhdr->nlmsg_type == RTM_NEWROUTE) {
        struct rtmsg * route;
        struct rtattr * rt_attr;
        int rta_length;
        printf("RTM_NEWROUTE\n");
        route = (struct rtmsg *) NLMSG_DATA(nlhdr);
        if (route->rtm_type == RTN_UNICAST) {
          printf("\ttype : unicast\n");
        } else if (route->rtm_type == RTN_UNSPEC) {
          printf("\ttype : unspecified\n");
        } else if (route->rtm_type == RTN_BROADCAST) {
          printf("\ttype : broadcast\n");
        } else if (route->rtm_type == RTN_LOCAL) {
          printf("\ttype : local\n");
        } else if (route->rtm_type == RTN_NAT) {
          printf("\ttype : NAT\n");
        }

        if (route->rtm_scope == RT_SCOPE_UNIVERSE) {
          printf("\tscope : global\n");
        } else if (route->rtm_scope == RT_SCOPE_SITE) {
          printf("\tscope : AS local\n");
        } else if (route->rtm_scope == RT_SCOPE_LINK) {
          printf("\tscope : link local\n");
        } else if (route->rtm_scope == RT_SCOPE_HOST) {
          printf("\tscope : host local\n");
        } else if (route->rtm_scope == RT_SCOPE_NOWHERE) {
          printf("\tscope : none (no destination)\n");
        }

        if (route->rtm_table == RT_TABLE_UNSPEC) {
          printf("\trouting table : unspecified\n");
        } else if (route->rtm_table == RT_TABLE_DEFAULT) {
          printf("\trouting table : default\n");
        } else if (route->rtm_table == RT_TABLE_MAIN) {
          printf("\trouting table : main\n");
        } else if (route->rtm_table == RT_TABLE_LOCAL) {
          printf("\trouting table : local\n");
        }

        rt_attr = RTM_RTA(route);
        rta_length = RTM_PAYLOAD(nlhdr);
        for (; rta_length && RTA_OK(rt_attr, rta_length); rt_attr = RTA_NEXT(rt_attr, rta_length)) {
          switch (rt_attr->rta_type) {
            case RTA_DST :
              printf("\t destination : %s", inet_ntoa((*(struct in_addr *)RTA_DATA(rt_attr))));
              break;
            case RTA_SRC :
              printf("\t source : %s", inet_ntoa((*(struct in_addr *)RTA_DATA(rt_attr))));
              break;
            case RTA_IIF :
              printf("\t input interface : %u", (*(uint32_t*)RTA_DATA(rt_attr)));
              break;
            case RTA_OIF :
              printf("\t output interface : %u", (*(uint32_t*)RTA_DATA(rt_attr)));
              break;
            default :
              break;
          }
        }
        printf("\n");
      }

      if (nlhdr->nlmsg_type == RTM_DELADDR) {
        printf("RTM_DELADDR\n");
      }

      if (nlhdr->nlmsg_type == RTM_DELROUTE) {
        printf("RTM_DELROUTE\n");
      }
    }

    printf("Message parsed\n\n");
  }
}
