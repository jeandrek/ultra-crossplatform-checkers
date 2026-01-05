	.toc

.text
	.globl gluPerspective
	.globl .gluPerspective
	.csect gluPerspective[DS]
gluPerspective:
.long .gluPerspective, TOC[tc0], 0
.text
.gluPerspective:
	blr

