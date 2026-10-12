#!/usr/bin/env python3
"""Graba el audio que la Nucleo manda por UART (OUTPUT_MODE = MODE_UART_RAW en main.c) a un .wav.

Uso: python3 tools/record_wav.py [segundos] [salida.wav]   (los dos son opcionales)
Sin segundos (o con 0) espera a que se apriete B1 y graba hasta que se vuelva a apretar.
Formato del stream: bloques [0xA5 0x5A seq] + 512 muestras int16 little-endian.
"""
import fcntl, glob, os, struct, sys, termios, time, wave

BAUD = 500000
RATE = 16000          # el real es 15.957 kHz (-0.26 %), irrelevante para escuchar
N = 512
SYNC = b'\xa5\x5a'  # marca de principio de bloque (BLOCK_SYNC_1/2 en app.c)
BLOCK = len(SYNC) + 1 + N * 2  # marca + seq + muestras int16
IOSSIOSPEED = 0x80045402  # macOS: baud arbitrario


def open_port():
    # macOS: /dev/cu.usbmodem*; Linux: /dev/ttyACM* (el ST-Link aparece como CDC ACM)
    ports = glob.glob('/dev/cu.usbmodem*') + glob.glob('/dev/ttyACM*')
    if not ports:
        sys.exit('No encuentro el puerto serie de la Nucleo. Esta enchufada?')
    fd = os.open(ports[0], os.O_RDONLY | os.O_NOCTTY | os.O_NONBLOCK)
    a = termios.tcgetattr(fd)
    a[0] = a[1] = a[3] = 0
    a[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    # 500000 es una velocidad estandar en Linux pero no en macOS, donde se fija aparte
    linux = hasattr(termios, 'B500000')
    a[4] = a[5] = termios.B500000 if linux else termios.B115200
    termios.tcsetattr(fd, termios.TCSANOW, a)
    if not linux:
        fcntl.ioctl(fd, IOSSIOSPEED, struct.pack('I', BAUD))
    termios.tcflush(fd, termios.TCIFLUSH)
    return fd


def parse(buf):
    """Devuelve (bloques [(seq, bytes)], resto). Un header solo vale si el siguiente
    header aparece justo BLOCK bytes despues (los datos pueden contener A5 5A)."""
    out, i = [], 0
    while True:
        j = buf.find(SYNC, i)
        if j < 0 or j + BLOCK + 2 > len(buf):
            return out, buf[max(j, 0):] if j >= 0 else buf[-1:]
        if buf[j + BLOCK:j + BLOCK + 2] == SYNC:
            out.append((buf[j + 2], buf[j + 3:j + BLOCK]))
            i = j + BLOCK
        else:
            i = j + 1


def main():
    args = sys.argv[1:]
    try:
        secs = float(args[0]) if args else 0
        args = args[1:]
    except ValueError:  # el primer argumento es el archivo, no los segundos
        secs = 0
    STOP_GAP = 1.0  # sin segundos fijos: corta cuando no llega audio durante este tiempo
    default = time.strftime('recordings/rec_%Y%m%d_%H%M%S.wav')
    path = args[0] if args else default
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)

    fd = open_port()
    buf, pcm, last_seq, lost, synced = b'', bytearray(), None, 0, False
    if not secs:
        print('Esperando audio: apreta B1 para empezar y de nuevo para terminar...')
    end = time.time() + secs if secs else None
    last_data = None
    while True:
        now = time.time()
        if end is not None and now >= end:
            break
        if end is None and pcm and now - last_data > STOP_GAP:
            break
        try:
            d = os.read(fd, 4096)
        except BlockingIOError:  # macOS: no hay datos
            d = b''
        if not d:  # Linux devuelve 0 bytes en vez de la excepcion
            time.sleep(0.005)
            continue
        last_data = now
        buf += d
        blocks, buf = parse(buf)
        for seq, data in blocks:
            # Al abrir el puerto pueden llegar primero bloques viejos que quedaron en el
            # buffer del ST-Link: se graba recien despues de dos bloques consecutivos
            if not synced:
                synced = last_seq is not None and (seq - last_seq) & 0xFF == 1
                last_seq = seq
                continue
            lost += (seq - last_seq - 1) & 0xFF
            last_seq = seq
            pcm += data
    os.close(fd)

    if not pcm:
        sys.exit('No llegaron bloques. Esta flasheado con OUTPUT_MODE = MODE_UART_RAW? '
                 'Hay otro programa con el puerto abierto?')

    with wave.open(path, 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(bytes(pcm))

    samples = struct.unpack('<%dh' % (len(pcm) // 2), bytes(pcm))
    peak = max(abs(x) for x in samples)
    print('%s: %.1f s, %d muestras, pico=%d/32767, bloques perdidos=%d'
          % (path, len(samples) / RATE, len(samples), peak, lost))


if __name__ == '__main__':
    main()
