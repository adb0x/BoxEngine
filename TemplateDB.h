#ifndef TEMPLATEDB_H
#define TEMPLATEDB_H

#include <string>
#include <unordered_map>
#include <rapidjson/document.h>

class TemplateDB {
public:
    static bool LoadTemplates();
    static bool HasTemplate(const std::string& name);
    static const rapidjson::Document& GetTemplate(const std::string& name);

private:
    static std::unordered_map<std::string, rapidjson::Document> templates;
};

#endif