#ifndef CDDF2A0C_9649_4D49_BFF9_30A871EA8630
#define CDDF2A0C_9649_4D49_BFF9_30A871EA8630

#include <iosfwd>

namespace po {
class file;
}  // namespace po

void process_file(const po::file& po, std::ostream& json);

#endif /* CDDF2A0C_9649_4D49_BFF9_30A871EA8630 */
