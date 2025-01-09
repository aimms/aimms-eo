FROM almalinux:8.9
LABEL maintainer="AIMMS <support@aimms.com>"
ARG AIMMS_VERSION

#########################
#      base image       #
#########################
RUN dnf upgrade -y almalinux-release

RUN dnf -y update \ 
    && dnf -y install  \
            gcc-toolset-11-runtime \
            gcc-gfortran \
            unixODBC \ 
            ca-certificates \
            gnupg \
            openssl \
            glibc-langpack-en \
            glibc-locale-source \
            dos2unix \
            wget \
            curl \
			python3.11 \
    && rm -f /etc/odbcinst.ini \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8 \
    && dnf clean all 
RUN    echo "source scl_source enable gcc-toolset-11" > /etc/profile.d/enable_gcc_toolset_11.sh \
    && chmod a+rx /etc/profile.d/enable_gcc_toolset_11.sh

COPY ./aimms_install.py /aimms_install.py
RUN python3 aimms_install.py --version ${AIMMS_VERSION}

VOLUME /data
VOLUME /model

COPY ./docker-entry.sh /docker-entry.sh
RUN chmod a+rx /docker-entry.sh && dos2unix /docker-entry.sh

ENTRYPOINT ["/docker-entry.sh"]

ENV LD_LIBRARY_PATH="/usr/local/Aimms/Bin"
