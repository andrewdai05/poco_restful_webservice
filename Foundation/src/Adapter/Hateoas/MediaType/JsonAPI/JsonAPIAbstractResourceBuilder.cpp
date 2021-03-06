#include "Adapter/Hateoas/MediaType/JsonAPI/JsonAPIAbstractResourceBuilder.h"

namespace Hateoas {


    JsonAPIAbstractResourceBuilder::JsonAPIAbstractResourceBuilder()
        : links(),
          resourceRootURL(),
          metadata(),
          attributes(),
          structRoot(),
          resourceBasic(),
          dataArrayRoot(),
          _buildAsBasicRepresentationOnly(false)
    {}

    Poco::DynamicStruct & JsonAPIAbstractResourceBuilder::build()
    {
        if ( !links.empty() )
            resourceBasic.insert("links", links);

        if ( !attributes.empty() )
            resourceBasic.insert("attributes", attributes);

        if (_buildAsBasicRepresentationOnly)
            return resourceBasic;

        dataArrayRoot.emplace_back(resourceBasic);

        if ( !metadata.empty() )
            structRoot.insert("meta", metadata);

        structRoot.insert("data", dataArrayRoot);
        return structRoot;
    }

    void JsonAPIAbstractResourceBuilder::buildAsPartialRepresentation(bool flag)
    {
        _buildAsBasicRepresentationOnly = flag;
    }

    Poco::Dynamic::Array & JsonAPIAbstractResourceBuilder::arrayRootObject()
    {
        return dataArrayRoot;
    }

    Poco::DynamicStruct & JsonAPIAbstractResourceBuilder::structRootObject()
    {
        return structRoot;
    }

    void JsonAPIAbstractResourceBuilder::hostedAt(const std::string & host)
    {
        resourceRootURL = host;
    }

    void JsonAPIAbstractResourceBuilder::withType(const std::string & type)
    {
        resourceBasic["type"] = type;
    }

    void JsonAPIAbstractResourceBuilder::withIdentity(const std::string & identity)
    {
        resourceBasic["id"] = identity;
    }

    void JsonAPIAbstractResourceBuilder::linkTo(const std::string & key, const std::string & value)
    {
        links[key] = "http://" + resourceRootURL + value;
    }

    void JsonAPIAbstractResourceBuilder::linkTo(const std::string & key, const Poco::DynamicStruct & buildingStruct)
    {
        links[key] = buildingStruct;
    }

    void JsonAPIAbstractResourceBuilder::withMetadata(const std::string & key, const std::string & value)
    {
        metadata.insert(key, value);
    }

    void JsonAPIAbstractResourceBuilder::withMetadata(const std::string & key, const Poco::Dynamic::Array & value)
    {
        metadata.insert(key, value);
    }

    void JsonAPIAbstractResourceBuilder::addAttribute(const std::string & key, const std::string & value)
    {
        attributes[key] = value;
    }


}
