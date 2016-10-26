all: reserva_bol_ser reserva_bol_cli

reserva_bol_cli: proyecto1_cliente.c
	gcc proyecto1_cliente.c -o reserva_bol_cli -Wformat-security

reserva_bol_ser: proyecto1_servidor.c
	gcc proyecto1_servidor.c -o reserva_bol_ser -lpthread -Wformat-security