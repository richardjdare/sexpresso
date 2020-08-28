# container project for sexpresso and sexpresso tests
message(sexpresso-project.pro)
TEMPLATE = subdirs
sexpresso.file = sexpresso.pro
sexpresso-tests.file = sexpresso-tests/sexpresso-tests.pro
SUBDIRS = \
    sexpresso \
    sexpresso-tests \

sexpresso-tests.depends = sexpresso
