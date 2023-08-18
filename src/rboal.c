extern void Init_al(void);
extern void Init_alc(void);
extern void Init_alut(void);

void Init_openal(void)
{
    Init_al();
    Init_alc();
//    Init_alut();
}
