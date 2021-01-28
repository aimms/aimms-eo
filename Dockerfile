FROM ubuntu:bionic-20200219
MAINTAINER AIMMS <support@aimms.com>
ARG AIMMS_VERSION_MAJOR
ARG AIMMS_VERSION_MINOR

RUN apt-get update && \
    apt-get install -y locales unixodbc wget gcc dos2unix xz-utils && \
    rm -rf /var/lib/apt/lists/* && \
    localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

RUN wget -q https://download.aimms.com/aimms/download/data/${AIMMS_VERSION_MAJOR}/${AIMMS_VERSION_MINOR}/Aimms-${AIMMS_VERSION_MAJOR}.${AIMMS_VERSION_MINOR}-installer.run && \
    chmod a+rx Aimms-$AIMMS_VERSION_MAJOR.$AIMMS_VERSION_MINOR-installer.run && \
    ./Aimms-$AIMMS_VERSION_MAJOR.$AIMMS_VERSION_MINOR-installer.run --target /usr/local/Aimms --noexec && \
	rm -rf /usr/local/Aimms/WebUIDev && \
    rm -f ./Aimms-$AIMMS_VERSION_MAJOR.$AIMMS_VERSION_MINOR-installer.run

VOLUME /data
VOLUME /model

COPY ./docker-entry.sh /docker-entry.sh
RUN chmod a+rx /docker-entry.sh && dos2unix /docker-entry.sh

ENTRYPOINT ["/docker-entry.sh"]
CMD ["jobrunner"]

COPY jobrunner.c /tmp/jobrunner.c

RUN gcc -I /usr/local/Aimms/Api /tmp/jobrunner.c -L /usr/local/Aimms/Bin -laimms3 -Wl,--hash-style=both -Wl,-R,'$ORIGIN/../Bin' -Wl,-R,'$ORIGIN/../Solvers' -o /usr/local/Aimms/Bin/jobrunner


