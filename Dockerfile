FROM gitpod/workspace-full-vnc:latest

USER root

RUN apt-get -y update && apt-get install -y --no-install-recommends \
      qtbase5-dev				\
      libqt5opengl5-dev				\
      libboost-dev				\
      libboost-regex-dev			\
      libboost-system-dev			\
      libboost-program-options-dev		\
      libeigen3-dev				\
      libmysqlclient-dev			\
      libmysql++-dev				\
      libsqlite3-dev				\
      liblog4cpp5-dev				\
      libgdal-dev				\
      libtinyxml2-dev				\
      libarchive-dev				\
      libjsoncpp-dev				\
      libtbb-dev			     && \
      rm -rf /var/lib/apt/lists/*
