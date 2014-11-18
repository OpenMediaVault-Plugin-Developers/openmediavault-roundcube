/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

Ext.define("OMV.module.admin.service.roundcube.Settings", {
    extend : "OMV.workspace.form.Panel",

    plugins: [{
        ptype        : "linkedfields",
        correlations : [{
            name       : [
                "installdb"
            ],
            conditions : [{
                name  : "enable",
                value : false
            }],
            properties : "!disabled"
        },{
            conditions  : [
                { name : "enable", value : true }
            ],
            properties : function(valid, field) {
                this.setButtonDisabled("webmail", !valid);
            }
        }]
    }],

    initComponent : function () {
        var me = this;

        me.on('load', function () {
            var checked = me.findField('enable').checked;
            var showtab = me.findField('showtab').checked;
            var parent = me.up('tabpanel');

            if (!parent)
                return;

            var webmailPanel = parent.down('panel[title=' + _("Webmail") + ']');

            if (webmailPanel) {
                checked ? webmailPanel.enable() : webmailPanel.disable();
                showtab ? webmailPanel.tab.show() : webmailPanel.tab.hide();
            }
        });

        me.callParent(arguments);
    },

    rpcService   : "Roundcube",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    getButtonItems : function() {
        var me = this;
        var items = me.callParent(arguments);
        items.push({
            id       : me.getId() + "-webmail",
            xtype    : "button",
            text     : _("Open Webmail"),
            icon     : "images/roundcube.png",
            iconCls  : Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled : true,
            scope    : me,
            handler  : function() {
                window.open("/webmail/", "_blank");
            }
        });
        return items;
    },

    getFormItems : function() {
        return [{
            xtype    : "fieldset",
            title    : _("General settings"),
            defaults : {
                labelSeparator:""
            },
            items    : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : false
            },{
                xtype      : "textfield",
                name       : "default_host",
                fieldLabel : _("Default Host"),
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("The mail host chosen to perform the log-in. Leave blank to show a textbox at login.")
                }]
            },{
                xtype      : "textfield",
                name       : "smtp_server",
                fieldLabel : _("SMTP Server"),
                allowBlank : false
            },{
                xtype      : "numberfield",
                name       : "smtp_port",
                fieldLabel : _("SMTP Port"),
                inputValue : 25
            },{
                xtype      : "textfield",
                name       : "smtp_user",
                fieldLabel : _("SMTP User"),
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("If required. Use %u as the username and Roundcube will use the current username for login.")
                }]
            },{
                xtype      : "textfield",
                name       : "smtp_pass",
                fieldLabel : _("SMTP Password"),
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("If required. Use %p as the password and Roundcube will use the current user's password for login.")
                }]
            },{
                xtype      : "textfield",
                name       : "product_name",
                fieldLabel : _("Product Name"),
                allowBlank : false
            },{
                xtype      : "passwordfield",
                name       : "des_key",
                fieldLabel : _("DES Key"),
                allowBlank : false,
                minLength  : 24,
                maxLength  : 24,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Random key created at installation.  Must be 24 characters long.")
                }]
            },{
                xtype      : "checkbox",
                name       : "showtab",
                fieldLabel : _("Show Tab"),
                boxLabel   : _("Show tab containing webmail frame."),
                checked    : false
            }]
        },{
            xtype    : "fieldset",
            title    : _("Database Settings"),
            defaults : {
                labelSeparator : ""
            },
            items    : [{
                xtype      : "textfield",
                name       : "db_host",
                fieldLabel : _("Hostname"),
                allowBlank : false
            },{
                xtype      : "textfield",
                name       : "db_name",
                fieldLabel : _("Database Name"),
                allowBlank : false
            },{
                xtype      : "textfield",
                name       : "db_user",
                fieldLabel : _("Username"),
                allowBlank : false
            },{
                xtype      : "passwordfield",
                name       : "db_pass",
                fieldLabel : _("Password"),
                allowBlank : false
            },{
                xtype      : "passwordfield",
                name       : "root_pass",
                fieldLabel : _("MySQL root Password"),
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Used only for installing Roundcube database and will not be saved.")
                }]
            },{
                xtype   : "button",
                name    : "installdb",
                text    : _("Install DB"),
                scope   : this,
                handler : function() {
                    var me = this;
                    OMV.MessageBox.show({
                        title   : _("Confirmation"),
                        msg     : _("Are you sure you want to install the Roundcube database?"),
                        buttons : Ext.Msg.YESNO,
                        fn      : function(answer) {
                            if (answer !== "yes")
                               return;

                            OMV.MessageBox.wait(null, _("Installing Roundcube database"));
                            OMV.Rpc.request({
                                scope   : me,
                                rpcData : {
                                    service : "Roundcube",
                                    method  : "doInstallDB",
                                    params  : {
                                        db_host   : me.getForm().findField("db_host").getValue(),
                                        db_name   : me.getForm().findField("db_name").getValue(),
                                        db_user   : me.getForm().findField("db_user").getValue(),
                                        db_pass   : me.getForm().findField("db_pass").getValue(),
                                        root_pass : me.getForm().findField("root_pass").getValue()
                                    }
                                },
                                success : function(id, success, response) {
                                    me.doReload();
                                    OMV.MessageBox.hide();
                                }
                            });
                        },
                        scope : me,
                        icon  : Ext.Msg.QUESTION
                    });
                },
                margin  : "5 0 8 0"
            }]
        }];
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/roundcube",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.admin.service.roundcube.Settings"
});
