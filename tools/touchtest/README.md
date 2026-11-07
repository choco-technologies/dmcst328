# touchtest

Prints what an input node reports (the generic input interface of dmdrvi, `DMDRVI_IOCTL_INPUT_*`) - e.g. the touch panel of `dmcst328`:

```
touchtest /dev/dmcst3280                         # the contacts as text
touchtest /dev/dmcst3280 /dev/dmst77890 30       # and an orange square under the finger on the display, for 30 s
```

It prints the device (`GET_INFO`), then waits for events (`WAIT_EVENT`) and prints every state (`GET_STATE`): the contacts with
their event (`down`, `move`, `up`), x, y (screen coordinates) and pressure. With a display node a square is drawn at every contact and
cleared when the finger is lifted. Touch the corners: the numbers must be the corners of the screen (`0,0` top left).
