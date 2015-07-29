#include <boost/test/unit_test.hpp>
#include "test_utils.hpp"

#include <owlapi/model/OWLOntology.hpp>
#include <owlapi/model/OWLOntologyReader.hpp>
#include <owlapi/model/OWLOntologyAsk.hpp>

using namespace owlapi::model;

BOOST_AUTO_TEST_SUITE(ontology)

BOOST_AUTO_TEST_CASE(load_restrictions)
{
    OWLOntologyReader reader;
    OWLOntology::Ptr ontology = reader.fromFile( getRootDir() + "/test/data/om-schema-v0.6.owl");
    OWLOntologyAsk ask(ontology);

    {
        std::vector<IRI> klasses;
        IRI sherpa("http://www.rock-robotics.org/2014/01/om-schema#Sherpa");
        klasses.push_back(sherpa);
        std::vector<OWLCardinalityRestriction::Ptr> restrictions = ask.getCardinalityRestrictions(klasses);
        BOOST_REQUIRE(restrictions.size() > 0);

        std::vector<OWLCardinalityRestriction::Ptr>::const_iterator cit = restrictions.begin();
        for(; cit != restrictions.end(); ++cit)
        {
            BOOST_TEST_MESSAGE("Restriction: " << (*cit)->toString());
        }
    }
    {
        IRI iri("http://www.rock-robotics.org/2014/01/om-schema#PayloadCamera");
        std::vector<OWLCardinalityRestriction::Ptr> restrictions = ask.getCardinalityRestrictions(iri);
        BOOST_CHECK_MESSAGE(restrictions.size() > 2, "PayloadCamera should inherit restrictions");

        std::vector<OWLCardinalityRestriction::Ptr>::const_iterator cit = restrictions.begin();
        for(; cit != restrictions.end(); ++cit)
        {
            BOOST_TEST_MESSAGE("Restrictions: " << (*cit)->toString());
        }
    }
}

BOOST_AUTO_TEST_CASE(punning)
{
    OWLOntologyReader reader;
    OWLOntology::Ptr ontology = reader.fromFile( getRootDir() + "/test/data/om-schema-v0.9.owl");
    OWLOntologyAsk ask(ontology);

    using namespace owlapi::model;
    {
        IRI actor = owlapi::vocabulary::OM::resolve("Sherpa");
        IRI property = owlapi::vocabulary::OM::resolve("maxVelocity");

        OWLLiteral::Ptr value = ask.getDataValue(actor, property);

        BOOST_REQUIRE_MESSAGE(value, "Data value of " << actor.toString() << " for " << property.toString() << " is found: " << value->getDouble());
    }
    {
        IRI actor = owlapi::vocabulary::OM::resolve("Sherpa");
        IRI property = owlapi::vocabulary::OM::resolve("probabilityOfFailure");

        OWLLiteral::Ptr value = ask.getDataValue(actor, property);

        BOOST_REQUIRE_MESSAGE(value, "Data value of " << actor.toString() << " for " << property.toString() << " is found: " << value->getDouble());
    }
    {
        IRI actor = owlapi::vocabulary::OM::resolve("Sherpa");
        IRI property = owlapi::vocabulary::OM::resolve("notExistingProperty");

        BOOST_REQUIRE_THROW(ask.getDataValue(actor, property), std::invalid_argument);
    }
}

BOOST_AUTO_TEST_SUITE_END()
