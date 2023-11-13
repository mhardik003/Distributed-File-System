client:
	# sudo lsof -i tcp:8081 | awk 'NR!=1 {print $2}' | xargs kill
	gcc -o Client\ Server/client Client\ Server/main.c
	./Client\ Server/client
naming_server:
	# sudo lsof -i tcp:8080 | awk 'NR!=1 {print $2}' | xargs kill
	gcc -o Naming\ Server/naming_server Naming\ Server/operation_handler.c Naming\ Server/main.c
	./Naming\ Server/naming_server
storage_server:
	# sudo lsof -i tcp:8082 | awk 'NR!=1 {print $2}' | xargs kill
	gcc -o Storage\ Server/storage_server Storage\ Server/main.c
	./Storage\ Server/storage_server
make clean:
	rm Client\ Server/client Naming\ Server/naming_server