<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.10
//
// <auto-generated>
//
// Generated from file `Service.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

namespace
{
    require_once 'Ice/BuiltinSequences.php';
    require_once 'IceLocal/CommunicatorF.php';
}

namespace IceBox
{
    global $IceBox__t_FailureException;
    class FailureException extends \Ice\LocalException
    {
        public function __construct($reason='')
        {
            $this->reason = $reason;
        }

        public function ice_id()
        {
            return '::IceBox::FailureException';
        }

        public function __toString(): string
        {
            global $IceBox__t_FailureException;
            return IcePHP_stringifyException($this, $IceBox__t_FailureException);
        }

        public $reason;
    }
    global $IcePHP__t_string;

    $IceBox__t_FailureException = IcePHP_defineException('::IceBox::FailureException', '\\IceBox\\FailureException', false, null, array(
        array('reason', $IcePHP__t_string, false, 0)));
}

namespace IceBox
{
    global $IceBox__t_Service;
    interface Service
    {
        public function start($name, $communicator, $args);
        public function stop();
    }
    $IceBox__t_Service = IcePHP_defineClass('::IceBox::Service', '\\IceBox\\Service', -1, false, true, null, null);
}
?>
