COPTS=-Wall -Werror -g

all: ssm_client ssm_server

ssm_client: ssm_client.c
	gcc ${COPTS} -o ssm_client ssm_client.c

ssm_server: ssm_server.c
	gcc ${COPTS} -o ssm_server ssm_server.c

.PHONY: clean

clean:
	rm ssm_client ssm_server
