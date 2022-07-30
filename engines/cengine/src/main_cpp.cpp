
extern "C" int dd_main(int argc, char *argv[]);

#ifdef __cplusplus
extern "C"
#endif
#if defined(_WIN32) || defined(WIN32)
int SDL_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
	return dd_main(argc, argv);
}
