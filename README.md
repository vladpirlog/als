ALS (Ambient Light Sensor) Driver
===

ASUS Zenbook Ambient Light Sensor Driver

Device `/sys/bus/acpi/devices/ACPI0008:00` (ACPI path: `_SB.ALS`)

### Exported attributes
- ali (Ambient Light Illuminance) (ACPI path: `_SB.ALS._ALI`)
- enable (read current enabled status or write 1 or 0 to enable/disable the sensor) (ACPI path: `_SB.ALAE` for read, `_SB.PCI0.LPCB.EC0.ALSC` for write)

### Stack
```
Method (_ALI, 0, NotSerialized)  // _ALI: Ambient Light Illuminance
{
    Return (^^PCI0.LPCB.EC0.RALS ())
}
```

```
Method (ALSC, 1, NotSerialized)
{
    If (Arg0)
    {
        TALS (One)
        Local0 = RALS ()
    }
    Else
    {
        TALS (Zero)
        Local0 = 0x0190
    }

    ALAE = Arg0
    If ((MSOS () == OSW7))
    {
        ^^^GFX0.AINT (Zero, Local0)
    }
    Else
    {
        Notify (ALS, 0x80) // Status Change
    }

    Return (One)
}
```

```
Method (TALS, 1, NotSerialized)
{
    If (Arg0)
    {
        Local0 = ST8E (0x30, Zero)
        Local0 |= 0x90
        ST9E (0x30, 0xFF, Local0)
    }
    Else
    {
        Local0 = ST8E (0x30, Zero)
        Local0 &= 0x6F
        ST9E (0x30, 0xFF, Local0)
    }
}
```

```
Method (RALS, 0, NotSerialized)
{
    If (ALAE)
    {
        Local0 = RRAM (0x02A3)
        Local1 = RRAM (0x02A4)
        Local0 <<= 0x08
        Local0 += Local1
        Local1 = (Local0 * 0x03E8)
        Divide (Local1, ALSA, Local2, Local3)
        Return (Local3)
    }
    Else
    {
        Return (0x0190)
    }
}
```
