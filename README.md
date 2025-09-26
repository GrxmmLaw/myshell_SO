# noha_shell

Consola de Linux desarrollada para el curso de Sistemas Operativos.

## Integrantes
- Ignacio Barria
- Franco Vidal
- Iván Zapata

---

## Compilación

Para compilar la consola, abre una terminal en la carpeta del proyecto y ejecuta:

```sh
gcc noha_sell.c -o noha_shell
```
## Ejecución

Para iniciar la consola, ejecuta:

```sh
./noha_shell
```

Verás el prompt personalizado `noha_shell:$` donde puedes ingresar comandos como en una shell normal.

---

## Uso de comandos

Puedes ejecutar comandos estándar de Linux, usar pipes (`|`) y el comando especial `miprof`.

---

## Comando especial: miprof

`miprof` permite ejecutar comandos mostrando información de tiempo y memoria, y tiene tres modos de uso:

### 1. Mostrar perfil por pantalla

```sh
miprof ejec comando argumentos
```

Ejemplo:
```sh
miprof ejec ls -l
```

### 2. Guardar perfil en archivo

```sh
miprof ejecsave archivo comando argumentos
```

Ejemplo:
```sh
miprof ejecsave salida.txt ls -l
```

El resultado se agrega al final del archivo especificado.

### 3. Ejecutar con límite de tiempo

```sh
miprof maxtiempo segundos comando argumentos
```

Ejemplo:
```sh
miprof maxtiempo 5 sleep 10
```

Si el comando excede el tiempo indicado, se termina y se muestra un mensaje de error.

### 4. Ayuda

Para ver la ayuda del comando miprof:

```sh
miprof -h
```

---

## Salida del comando miprof

La salida incluye:
- Comando ejecutado
- Tiempo real de ejecución
- Tiempo de CPU en modo usuario y sistema
- Memoria máxima utilizada

---

## Salir de la consola

Para salir, escribe:

```sh
exit
```
