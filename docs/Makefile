all: font_preview

font_preview: .force
	../bin/piece -f help \
		| tail -n +2 \
		| awk '{print $$1}' \
		| while read font; do \
		../bin/piece -f $$font -o font/$${font}.png font/test.ans; \
		done

.force:
