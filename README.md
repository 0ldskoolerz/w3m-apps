# w3m-apps

Suite de aplicaciones gráficas minimalistas para el escritorio
**[W3M](https://github.com/0ldskoolerz/W3M)** (WM de X11 estilo Windows 3.x).
Cada app es un binario Xlib independiente con la misma estética del WM —
y la filosofía **busybox**: la GUI es una piel fina; el trabajo pesado
(lo copiar, mover, comprimir, listar procesos, evaluar) lo hacen los
**applets de busybox** invocados con `execvp` + argv (sin shell, sin
inyección).

## Aplicaciones

| Binario | Equivalente clásico | Qué hace | Applets que usa |
|---|---|---|---|
| `w3m-fm` | File Manager (Win 3.11) | explorar, **crear** carpeta, **copiar**, **cortar/pegar**, **renombrar**, **comprimir** (tar.gz), **descomprimir** (tar.gz/tgz/zip), **abrir** (navegar dirs / xdg-open), borrar | `ls`, `mkdir`, `cp -r`, `mv`, `rm -r`, `tar`, `unzip`, `basename`, `dirname` |
| `w3m-term` | — (el imprescindible) | terminal: ejecuta comandos vía `/bin/sh -c` con cwd propio, scroll, `clear`/`exit` | shell + todos |
| `w3m-task` | Task Manager (Win 95) | lista procesos con PID/usuario, memoria (`free -h`), **matar proceso** (`kill -9`) | `ps`, `free`, `kill` |
| `w3m-calc` | Calculator | teclado 4×4 + display; evalúa con **whitelist de caracteres** | `awk` |
| `w3m-notepad` | Notepad | editor de texto: abrir/guardar, cursor, flechas, multi-línea | I/O directo |

## Filosofía de diseño

1. **Una app = un trabajo** (Unix). El WM no incluye apps; estas son un
   paquete aparte y opcional.
2. **Busybox hace el trabajo**: ~1 applet, 0 reimplementaciones. La GUI
   solo orquesta argv y pinta el resultado. Sin `system()` con input
   del usuario — todo `execvp` con arrays.
3. **Look heredado**: bisel Win 3.x, paleta `grey70`/`navyblue`, fuente
   `fixed` — idéntico al WM.

## Compilar

```sh
sudo pacman -S base-devel libx11 busybox    # busybox opcional (hay fallback)
make
./w3m-fm
```

Sin busybox, la capa `applet.c` reintenta con el binario standalone
(`ls`, `cp`, ...) — en cualquier distro normal funciona igual.

## El ecosistema

- [W3M](https://github.com/0ldskoolerz/W3M) — el gestor de ventanas
- [w3m-net](https://github.com/0ldskoolerz/w3m-net) — suite de red
  estilo Trinux (ping, DNS, ARP, scan, sniff...), misma filosofía
  GUI-Xlib + busybox
- [w3m-linux](https://github.com/0ldskoolerz/w3m-linux) — distro
  completa con todo integrado

## Usar con W3M

```sh
# en tu sesión W3M (o Xephyr de pruebas):
w3m-fm &
w3m-term &
```

## Estructura

```
src/
  ui.h / ui.c        primitivas compartidas (bevel, botones, colores, eventos)
  applet.h / .c      capa busybox: fork+execvp+pipe, captura de stdout
  fm.c               explorador de archivos
  term.c             terminal
  task.c             administrador de tareas
  calc.c             calculadora
  notepad.c          editor de texto
tests/x11_stub.h     stub Xlib para verificación sin headers X11
```

## Limitaciones actuales (MVP)

- El terminal no emula secuencias de escape completas (apto para shell
  y salida de comandos; no curses/vim — para eso, un xterm real)
- Renombrar en el FM crea `.ren` (renombrado rápido básico)
- Notepad: un archivo a la vez, sin scroll de viewport más allá de 22 líneas
