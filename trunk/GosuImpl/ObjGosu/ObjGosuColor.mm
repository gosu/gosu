//
//  ObjGosuColor.mm
//  Gosu
//
//  Created by Julian Raschke on 04.10.09.
//  Copyright 2009 Raschke & Ludwig GbR. All rights reserved.
//

#import "ObjGosuColor.h"


@implementation ObjGosuColor
- (boost::uint8_t)red
{
    return Gosu::Color(rep).red();
}
@end
