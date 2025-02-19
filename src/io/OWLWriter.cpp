#include "OWLWriter.hpp"
#include <algorithm>

namespace owlapi {
namespace io {

OWLWriter::OWLWriter(const std::string& defaultFormat)
    : OWLIOBase(defaultFormat)
{}

OWLWriter::~OWLWriter()
{}

} // end namespace io
} // end namespace owlapi
