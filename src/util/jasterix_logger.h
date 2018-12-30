#ifndef JASTERIX_LOGGER_H
#define JASTERIX_LOGGER_H

#include "log4cpp/Appender.hh"
#include "log4cpp/Category.hh"

#define logerr log4cpp::Category::getRoot().errorStream()
#define logwrn log4cpp::Category::getRoot().warnStream()
#define loginf log4cpp::Category::getRoot().infoStream()

#endif // JASTERIX_LOGGER_H
