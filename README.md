# About this Repo

This is the Git repo of the Docker jobrunner image for the [AIMMS](https://www.aimms.com) Embedded Optimization service.

## DOCKER COMPOSE

To run aimms unit tests easily with docker compose, you can use the following command:

```bash
docker compose run --rm --build aimms
```

This will take the docker-compose.yml file and build and run AIMMS. If you want Different Aimms Versions you can change the .env file which has the AIMMS_VERSION in it.

This is a simple way to run aimms unit tests within a docker container in a pipeline.

# Deployment options

The Dockerfile defines two volumes to be present were the license configuration and the AIMMS model should be present. This provides a way to centralized administration of the AIMMS application(s) (versions) and their licenses. The implication is however that that volume should be available to every node in your cluster, which might require some additional setup to your cluster.

An alternative to that is to not use volumes at all, but to package everything in a derived docker-image. This however would require you to publish a new version of that image with an updated license file every year.

Of course it's also possible to make a hybrid variant where e.g. the model is embedded into the container-image, but the license information is provided by a volume. Perhaps this offers most flexibility to quickly update licenses when needed and benefit from some performance gains when the AIMMS model is contained in the image.

Also there is the choice to make between using an AimmsPack file and just the plain model folder. When using the AimmsPack the model and data are encrypted, but will be extracted by AIMMS at some point in order to execute the model properly. This can cause a small performance overhead for starting your model. It depends on your situation what is best for you. 

To discuss deployment options and the various pros and cons, you can contact support@aimms.com.

# Setting up the licenses
As part of the AIMMS Embedded Optimization service, AIMMS will provide you with a so called keyless license, that needs to be available for AIMMS to function properly. The keyless licenses consists of two files, similar to:

```
015120001005.cpx
015120001005.lic
```

These files need to go into a subfolder 'Licenses' of the /data volume. Next you need to specify to AIMMS which license(s) to use by creating a licenses.cfg file into the subfolder 'Config' of the /data volume. The content of the licenses.cfg should be similar to:

```
1 local 015120001005.lic
```
where the name of the .lic should correspond with the .lic AIMMS provided. This should result in a file layout similar to:

```console
root@509c08b1faae:/# find /data
/data
/data/Config
/data/Config/licenses.cfg
/data/Licenses
/data/Licenses/015120001005.cpx
/data/Licenses/015120001005.lic
/data/Logging
/data/Nodelocks
```


# Connecting with external databases with AIMMS 24 and newer
Below are listed some usefull Dockerfile snippets for further specializing your docker image to use AIMMS in combination with ODBC database drivers.


## MySQL server ODBC
```dockerfile
## MySQL server ODBC
ENV MYSQL_EIGHT_ODBC_CONNECTOR_VERSION=mysql-connector-odbc-8.0.29-1.el8.x86_64.rpm
RUN cd /root \
    && wget https://cdn.mysql.com/archives/mysql-connector-odbc-8.0/${MYSQL_EIGHT_ODBC_CONNECTOR_VERSION} \
    && rpm -i ${MYSQL_EIGHT_ODBC_CONNECTOR_VERSION} \
    && myodbc-installer -d -a -n "MySQL8.0" -t "DRIVER=/usr/lib64/libmyodbc8w.so;" \
    && myodbc-installer -d -a -n "MySQL" -t "DRIVER=/usr/lib64/libmyodbc8w.so;" \
    && rm -f /root/${MYSQL_EIGHT_ODBC_CONNECTOR_VERSION}
```

## Microsoft SQL Server ODBC
```dockerfile
## Microsoft SQL Server ODBC
RUN    curl https://packages.microsoft.com/config/rhel/8/prod.repo > /etc/yum.repos.d/mssql-release.repo \
    && dnf -y update \
    && ACCEPT_EULA=Y dnf install -y msodbcsql18 \
    && myodbc-installer -d -a -n "MS SQL Server" -t "DRIVER=/opt/microsoft/msodbcsql18/lib64/libmsodbcsql-18.1.so.2.1;" \
    && rm -rf /var/cache \
    && dnf clean all 
```

## SQLite3 ODBC
```dockerfile
## SQLite3 ODBC
RUN    dnf -y update \ 
    && dnf -y install  \
              sqlite \
              sqlite-devel \
              unixODBC-devel \
    && cd /root \
    && wget http://www.ch-werner.de/sqliteodbc/sqliteodbc-0.9998.tar.gz \
    && tar xvfz sqliteodbc-0.9998.tar.gz \
    && cd sqliteodbc-0.9998 \
    && ./configure \
    && make install \
    && myodbc-installer -d -a -n "SQLite" -t "DRIVER=/usr/local/lib/libsqlite3odbc.so;" \
    && myodbc-installer -d -a -n "SQLite3" -t "DRIVER=/usr/local/lib/libsqlite3odbc.so;" \
    && cd .. \
    && rm -rf sqliteodbc-0.9998 \
    && rm -f sqliteodbc-0.9998.tar.gz \
    && dnf clean all 
```

## Oracle ODBC driver
```dockerfile
## Oracle ODBC driver
COPY ol8-temp.repo /etc/yum.repos.d/ol8-temp.repo
RUN wget https://yum.oracle.com/RPM-GPG-KEY-oracle-ol8 -O /etc/pki/rpm-gpg/RPM-GPG-KEY-oracle \
 && gpg --import --import-options show-only /etc/pki/rpm-gpg/RPM-GPG-KEY-oracle \
 && dnf -y install oraclelinux-release-el8 \
 && mv /etc/yum.repos.d/ol8-temp.repo /etc/yum.repos.d/ol8-temp.repo.disabled \
 && dnf -y update \
 && dnf -y install oracle-instantclient-release-el8 \
 && dnf -y install oracle-instantclient-odbc \
 && /usr/lib/oracle/21/client64/bin/odbc_update_ini.sh / /usr/lib/oracle/21/client64/lib \
 && dnf clean all \
 && rm -rf /var/cache
```

# Connecting with external databases for AIMMS versions prior to AIMMS 24:
Below are listed some usefull Dockerfile snippets for further specializing your docker image to use AIMMS in combination with ODBC database drivers.

## MySQL
```dockerfile
# install MySQL ODBC driver
ENV MYSQL_ODBC_CONNECTOR_VERSION=mysql-connector-odbc-5.3.14-linux-ubuntu18.04-x86-64bit
RUN cd /root \
    && wget https://dev.mysql.com/get/Downloads/Connector-ODBC/5.3/${MYSQL_ODBC_CONNECTOR_VERSION}.tar.gz \
    && tar xvzf ${MYSQL_ODBC_CONNECTOR_VERSION}.tar.gz \
    && cp /root/${MYSQL_ODBC_CONNECTOR_VERSION}/lib/* /usr/lib/x86_64-linux-gnu/odbc/ \
    && /root/${MYSQL_ODBC_CONNECTOR_VERSION}/bin/myodbc-installer -d -a -n "MySQL" -t "DRIVER=/usr/lib/x86_64-linux-gnu/odbc/libmyodbc5w.so;" \
    && rm -rf /root/${MYSQL_ODBC_CONNECTOR_VERSION}*
```

## Microsoft SQL Server
```dockerfile
# install Microsoft SQLServer ODBC driver
RUN cd /root \
    && curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add - \
    && curl https://packages.microsoft.com/config/ubuntu/18.04/prod.list > /etc/apt/sources.list.d/mssql-release.list \
    && apt-get update \
    && ACCEPT_EULA=Y apt-get install msodbcsql17 \
    && rm -rf /var/lib/apt/lists/*
```
