<?php
/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2013 Volker Theile
 * @copyright Copyright (C) 2013-2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceRoundcube extends OMVRpcServiceAbstract {

    private function getEventMessagePath()
    {
        return "org.openmediavault.services.roundcube";
    }

    private function getXPath()
    {
        return "//services/roundcube";
    }

    public function getName() {
        return "Roundcube";
    }

    public function initialize() {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");

        $this->registerMethod("doInstallDB");
    }

    public function getSettings($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Get configuration object
        $object = $xmlConfig->get($this->getXPath());
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        $object['enable']    = boolval($object['enable']);
        $object['smtp_port'] = intval($object['smtp_port']);
        $object['showtab']   = boolval($object['showtab']);

        return $object;
    }

    public function setSettings($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "enable"       : { "type" : "boolean" },
                "default_host" : { "type" : "string", "optional" : true },
                "smtp_server"  : { "type" : "string" },
                "smtp_port"    : { "type" : "integer" },
                "smtp_user"    : { "type" : "string", "optional" : true },
                "smtp_pass"    : { "type" : "string", "optional" : true },
                "product_name" : { "type" : "string" },
                "des_key"      : { "type" : "string", "minLength" : 24 },
                "db_host"      : { "type" : "string" },
                "db_user"      : { "type" : "string" },
                "db_pass"      : { "type" : "string" },
                "db_name"      : { "type" : "string" },
                "root_pass"    : { "type" : "string", "optional" : true },
                "showtab"      : { "type" : "boolean" }
            }
        }');

        // Trim des_key if longer than 24 characters
        if ( strlen($params['des_key']) > 24 )
            $params['des_key'] = substr($params['des_key'], 0, 24);

        // Prepare configuration data
        $object = array(
            'enable'       => array_boolval($params, 'enable'),
            'default_host' => $params['default_host'],
            'smtp_server'  => $params['smtp_server'],
            'smtp_port'    => $params['smtp_port'],
            'smtp_user'    => $params['smtp_user'],
            'smtp_pass'    => $params['smtp_pass'],
            'product_name' => $params['product_name'],
            'des_key'      => $params['des_key'],
            'db_host'      => $params['db_host'],
            'db_user'      => $params['db_user'],
            'db_pass'      => $params['db_pass'],
            'db_name'      => $params['db_name'],
            'showtab'      => array_boolval($params, 'showtab')
        );

        // Set configuration object
        if (FALSE === $xmlConfig->replace($this->getXPath(), $object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify general configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object);
    }

    public function doInstallDB($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "db_host"   : { "type" : "string" },
                "db_name"   : { "type" : "string" },
                "db_user"   : { "type" : "string" },
                "db_pass"   : { "type" : "string" },
                "root_pass" : { "type" : "string" }
            }
        }');

        // Get configuration object
        $object = $xmlConfig->get($this->getXPath());
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        $object['db_host'] = $params['db_host'];
        $object['db_name'] = $params['db_name'];
        $object['db_user'] = $params['db_user'];
        $object['db_pass'] = $params['db_pass'];

        // Set configuration object
        if (FALSE === $xmlConfig->replace($xpath, $object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        //create password file
        $file = "/tmp/.openmediavault-roundcube-mysql-pwd";
        file_put_contents($file, $params['root_pass']);

        // install database
        $cmd = "/usr/local/bin/create_roundcube_db 2>&1";
        if(0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));

        // Notify general configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object);
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRoundcube());

?>
