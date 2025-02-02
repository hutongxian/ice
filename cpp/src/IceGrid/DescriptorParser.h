//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_DESCRIPTOR_PARSER_H
#define ICEGRID_DESCRIPTOR_PARSER_H

namespace IceGrid
{

namespace DescriptorParser
{

ApplicationDescriptor parseDescriptor(const std::string&,
                                             const Ice::StringSeq&,
                                             const std::map<std::string, std::string>&,
                                             const std::shared_ptr<Ice::Communicator>&,
                                             const IceGrid::AdminPrxPtr&);

ApplicationDescriptor parseDescriptor(const std::string&, const std::shared_ptr<Ice::Communicator>&);

}

}

#endif
