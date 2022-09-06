extern void __files_init();
extern void __exiting_init();
extern void __memory_init();
extern void __random_init();
extern void __environment_init(char* env);

void __libc_init(const char* env)
{
    __memory_init();
    __random_init();
    __exiting_init();
    __files_init();
    __environment_init((char*)env);
}
