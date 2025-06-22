build_dir := build

.PHONY: all clean rebuild run test

all:
	@mkdir -p $(build_dir)
	cd $(build_dir) && cmake .. && make

build:
	@mkdir -p $(build_dir)
	cd $(build_dir) && cmake -DENABLE_TESTING=OFF .. && make

run: all
	./$(build_dir)/src/db_main

test: all
	cd $(build_dir) && ctest --output-on-failure --verbose

clean:
	rm -rf $(build_dir)

rebuild: clean all
