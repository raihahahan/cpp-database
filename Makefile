build_dir := build

.PHONY: all clean rebuild run

all:
	@mkdir -p $(build_dir)
	cd $(build_dir) && cmake .. && make

run: all
	./$(build_dir)/src/db_main

clean:
	rm -rf $(build_dir)

rebuild: clean all
