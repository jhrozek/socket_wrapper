#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "config.h"
#include "torture.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void setup_echo_srv_tcp_ipv4(void **state)
{
	torture_setup_echo_srv_tcp_ipv4(state);
}

static void teardown(void **state)
{
	torture_teardown_echo_srv(state);
}

static void test_connect_broadcast_ipv4(void **state)
{
	struct torture_address addr = {
		.sa_socklen = sizeof(struct sockaddr_in),
	};
	int rc;
	int s;

	(void) state; /* unused */

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert_int_not_equal(s, -1);

	addr.sa.in.sin_family = AF_INET;
	addr.sa.in.sin_port = htons(torture_server_port());
	addr.sa.in.sin_addr.s_addr = INADDR_BROADCAST;

	/* We don't allow connect to broadcast addresses */
	rc = connect(s, &addr.sa.s, addr.sa_socklen);
	assert_int_equal(rc, -1);

	close(s);
}

#ifdef HAVE_IPV6
static void test_connect_downgrade_ipv6(void **state)
{
	struct torture_address addr = {
		.sa_socklen = sizeof(struct sockaddr_in),
	};
	int rc;
	int s;

	(void) state; /* unused */

	s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	assert_int_not_equal(s, -1);

	addr.sa.in.sin_family = AF_INET;
	addr.sa.in.sin_port = htons(torture_server_port());

	rc = inet_pton(addr.sa.in.sin_family,
		       torture_server_address(AF_INET),
		       &addr.sa.in.sin_addr);
	assert_int_equal(rc, 1);

	/* Connect should downgrade to IPv4 and allow the connect */
	rc = connect(s, &addr.sa.in, addr.sa_socklen);
	assert_int_equal(rc, 0);

	close(s);
}
#endif

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test_setup_teardown(test_connect_broadcast_ipv4, setup_echo_srv_tcp_ipv4, teardown),
#ifdef HAVE_IPV6
		unit_test_setup_teardown(test_connect_downgrade_ipv6, setup_echo_srv_tcp_ipv4, teardown),
#endif
	};

	rc = run_tests(tests);

	return rc;
}
