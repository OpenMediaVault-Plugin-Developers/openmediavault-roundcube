<?php
/**
 *
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2013 Volker Theile
 * @copyright Copyright (C) 2013-2015 OpenMediaVault Plugin Developers
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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/initscript.inc");
require_once("openmediavault/log.inc");

class OMVModuleRoundcube extends OMVModuleServiceAbstract
    implements OMVINotifyListener
{

    private function getEventMessagePath()
    {
        return "org.openmediavault.services.roundcube";
    }

    private function getXPath()
    {
        return "//services/roundcube";
    }

    public function getName()
    {
        return "roundcube";
    }

    public function applyConfig()
    {
        $cmd = "omv-mkconf roundcube 2>&1";
        if (0 !== $this->exec($cmd, $output))
        {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($this, "setDirty")
        );

        $moduleMgr = &OMVModuleMgr::getInstance();
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($moduleMgr->getModule("php5fpm"), "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($moduleMgr->getModule("webserver"), "setDirty")
        );
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleRoundcube());
