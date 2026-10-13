#ifndef ROOM_FAN_SWEEP_SPARK_NAME
#error "define ROOM_FAN_SWEEP_SPARK_NAME and its parameters (COLOR, ANCHOR, TRAIL, COS, SIN, RAND, PALETTE_LIT, PALETTE_BASE) before including fan_sweep_spark.inc"
#endif

/* Fan sweep spark: circles the published anchor while it rises, dropping a
 * trail point now and then; it swells for 12 frames, holds for 12, then
 * fades while its radius grows. */
int ROOM_FAN_SWEEP_SPARK_NAME(int mode, RoomFanSweepSpark *spark) {
    GteShortVector position;
    GteRotation spin;
    RenderColor color = ROOM_FAN_SWEEP_SPARK_COLOR;
    GteShortVector *point;
    int size;
    int alpha;

    switch (mode) {
    case 1:
        spark->timer++;
        spark->y = g_FieldFxCurrentAge * 700 / 36 - 700 + ROOM_FAN_SWEEP_SPARK_ANCHOR.y;
        spark->x = ROOM_FAN_SWEEP_SPARK_ANCHOR.x + ROOM_FAN_SWEEP_SPARK_COS((s16)spark->angle) * spark->radius / 4096;
        spark->z = ROOM_FAN_SWEEP_SPARK_ANCHOR.z + ROOM_FAN_SWEEP_SPARK_SIN((s16)spark->angle) * spark->radius / 4096;
        spark->angle += 0x80;
        if (!(ROOM_FAN_SWEEP_SPARK_RAND() & 7)) {
            point = (GteShortVector *)FieldFx_AcquireCallbackSlot(ROOM_FAN_SWEEP_SPARK_TRAIL);
            if (point) {
                point->x = spark->x;
                point->y = spark->y;
                point->z = spark->z;
            }
        }
        if (g_FieldFxCurrentAge < 0x24) break;
        return 1;
    case 2:
        switch (spark->state) {
        case 0:
            size = ROOM_FAN_SWEEP_SPARK_SIN(((s16)spark->timer << 10) / 12);
            alpha = 0x40;
            if ((s16)spark->timer < 12) break;
            spark->timer = 0;
            spark->state = 1;
            break;
        case 1:
            size = 0x1000;
            alpha = 0x40;
            if ((s16)spark->timer < 12) break;
            spark->timer = 0;
            spark->state = 2;
            break;
        default:
            size = 0x1000;
            alpha = ROOM_FAN_SWEEP_SPARK_COS(((s16)spark->timer << 10) / 12) / 64;
            spark->radius += (s16)spark->timer * 4;
            break;
        }
        position.x = spark->x;
        position.y = spark->y;
        position.z = spark->z;
        size = size * 3 / 2;
        spin.x = 0;
        spin.y = 0;
        spin.z = g_FieldFxCurrentAge << 7;
        spin.flags = 0;
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            FieldFx_DrawShapeQuads(&position, &spin, size, size, 0x24,
                          GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + ROOM_FAN_SWEEP_SPARK_PALETTE_LIT : palette + ROOM_FAN_SWEEP_SPARK_PALETTE_BASE),
                          1, alpha, &color);
        }
        break;
    }
    return 0;
}
