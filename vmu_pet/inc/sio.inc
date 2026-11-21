;; Moemon VMU pet: VMU-to-VMU serial (Sega's VMU tutorial VMT-72..88 and hardware manual VMD-100..113, 229).
;; Each unit sends on SIO0 and listens on SIO1; the units' top connectors cross them. Sega's rules: only set the
;; serial port up while P7 says another VMU is there (P70 = 0, P72 = 0, P73 = 1), put it back (sio_end) when done or
;; when the cable goes. Every byte is echoed back by the other side, so the sender knows it arrived and never sends
;; faster than the other side reads (no overruns, no send-wait arithmetic).
;; Needs (RAM bank 1): ticks (+1 every half second), sio_t0, sio_t1, sio_speed (an SBR value).

SIO_SYNC = $4d              ; 'M': a packet starts
SIO_ACK  = $06
SIO_NAK  = $15

;; sio_connected: acc = 0 when another VMU is plugged into the top connector
sio_connected:
  ld p7
  and #%00001101
  sub #%00001000
  ret

;; sio_init: serial on, listening (call only when connected)
sio_init:
  mov #0, scon0             ; LSB first, 8-bit transfers
  mov #0, scon1
  ld sio_speed
  st sbr
  clr1 p1, 0
  clr1 p1, 2
  clr1 p1, 3
  mov #%00000101, p1fcr
  mov #%00000101, p1ddr
  mov #0, sbuf0
  mov #0, sbuf1
  set1 scon1, 3             ; listen
  ret

;; sio_end: the port back as the BIOS wants it
sio_end:
  mov #0, scon0
  mov #0, scon1
  mov #$bf, p1fcr
  mov #$a4, p1ddr
  ret

;; sio_send: acc = a byte (waits for the previous one to finish)
sio_send:
  push acc
.wait:
  ld scon0
  bp acc, 3, .wait
  pop acc
  st sbuf0
  set1 scon0, 3
  ret

;; sio_sent: wait until the last sio_send has gone out
sio_sent:
  push acc
.wait:
  ld scon0
  bp acc, 3, .wait
  pop acc
  ret

;; sio_poll: acc = 0 and b = the byte when one has arrived (already listening again), else acc = $ff
sio_poll:
  ld scon1
  bp acc, 1, .got
  bp acc, 3, .none
  set1 scon1, 3
.none:
  mov #$ff, acc
  ret
.got:
  ld sbuf1
  st b
  clr1 scon1, 1
  clr1 scon1, 6
  set1 scon1, 3
  mov #0, acc
  ret

;; sio_recv: wait about a second and a half for a byte: acc = 0 and b = it, or acc = $ff
sio_recv:
  ld ticks
  st sio_t0
.loop:
  call sio_poll
  bz .done
  ld ticks
  sub sio_t0
  sub #3
  bp psw, 7, .loop
  mov #$ff, acc
.done:
  ret

;; sio_xfer: send acc and check its echo: acc = 0 when it came back the same
sio_xfer:
  st sio_t1
  call sio_send
  call sio_recv
  bnz .fail
  ld b
  sub sio_t1
  ret
.fail:
  mov #$ff, acc
  ret

;; sio_flush: drop anything already received
sio_flush:
  call sio_poll
  bz sio_flush
  ret
