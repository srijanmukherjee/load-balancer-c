balancer: balancer.c
	gcc -o balancer balancer.c

init:
	yarn install

servers:
	node servers.js

clean: 
	rm -rf node_modules
	rm balancer