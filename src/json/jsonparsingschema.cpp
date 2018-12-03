#include "jsonparsingschema.h"
#include "jsonimportertask.h"

JSONParsingSchema::JSONParsingSchema(const std::string& class_id, const std::string& instance_id,
                                     JSONImporterTask& task)
    : Configurable(class_id, instance_id, &task), task_(&task)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

JSONParsingSchema& JSONParsingSchema::operator=(JSONParsingSchema&& other)
{

    name_ = other.name_;
    task_ = other.task_;

    parsers_ = std::move(other.parsers_);

    other.configuration().updateParameterPointer ("name", &name_);

//    widget_ = std::move(other.widget_);
//    if (widget_)
//        widget_->setParser(*this);
//    other.widget_ = nullptr;

    return static_cast<JSONParsingSchema&>(Configurable::operator=(std::move(other)));
}

void JSONParsingSchema::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "JSONObjectParser")
    {
        std::string name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("db_object_name");

        assert (parsers_.find (name) == parsers_.end());

        logdbg << "JSONParsingSchema: generateSubConfigurable: generating schema " << instance_id
               << " with name " << name;

        parsers_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(name),  // args for key
                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
    }
    else
        throw std::runtime_error ("JSONImporterTask: generateSubConfigurable: unknown class_id "+class_id );
}

std::string JSONParsingSchema::name() const
{
    return name_;
}

void JSONParsingSchema::name(const std::string &name)
{
    name_ = name;
}

JSONObjectParser& JSONParsingSchema::parser (const std::string& name)
{
    assert (hasObjectParser(name));
    return parsers_.at(name);
}
