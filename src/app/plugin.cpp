/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

/**
 * @file plugin.cpp
 * @author Wintermute Development <wntr-devel@thesii.org>
 *
 */

#include <QFile>
#include <QDebug>
#include <QApplication>

#include <app/core.hpp>
#include <app/config.hpp>
#include <app/factory.hpp>
#include <app/plugin.hxx>
#include <app/plugin.hpp>

using namespace Wintermute::Plugins;

AbstractPlugin::AbstractPlugin() : QObject(),
    d_ptr (new AbstractPluginPrivate (this))
{

}

AbstractPlugin::AbstractPlugin (QPluginLoader* p_pluginLoader) :
    QObject (p_pluginLoader), d_ptr (new AbstractPluginPrivate (this))
{
    Q_D (AbstractPlugin);
    d->m_plgnLdr = p_pluginLoader;
}

AbstractPlugin::AbstractPlugin (const AbstractPlugin& p_other) : QObject (p_other.parent()),
    d_ptr (const_cast<AbstractPluginPrivate*> (p_other.d_ptr.data()))
{
    Q_D (AbstractPlugin);
    d->q_ptr = this;
}

QString AbstractPlugin::author() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Description/Author").toString();
}

QString AbstractPlugin::name() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Description/Name").toString();
}

QString AbstractPlugin::vendorName() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Description/Vendor").toString();
}

QString AbstractPlugin::uuid() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Version/UUID").toString();
}

QString AbstractPlugin::description() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Description/Blurb").toString();
}

QString AbstractPlugin::webPage() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Description/WebPage").toString();
}

double AbstractPlugin::version() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Version/Plugin").toDouble();
}

double AbstractPlugin::compatVersion() const
{
    Q_D (const AbstractPlugin);
    return d->m_sttngs->value ("Version/Compat", WNTR_VERSION).toDouble();
}

bool AbstractPlugin::isSupported() const
{
    return WNTR_VERSION >= compatVersion();
}

QStringList AbstractPlugin::plugins() const
{
    Q_D (const AbstractPlugin);
    QStringList dep = d->m_sttngs->value ("Depends/Plugins").toStringList();
    dep.removeDuplicates();
    dep.removeAll ("None");
    return dep;
}

bool AbstractPlugin::hasPlugins() const
{
    const QStringList deps = this->plugins();
    foreach (const QString dep, deps) {
        const QString depName = dep.split (" ").at (0);
        const QString depComparison = dep.split (" ").at (1);
        const QString depVersion = dep.split (" ").at (2);

        if (Factory::allPlugins().contains (depName)) {
            if (!Factory::loadedPlugins().contains (depName))
                qDebug() << "(core) [AbstractPlugin] Dependency" << depName
                         << "of" << this->name () << "isn't loaded.";

            const AbstractPlugin* plgn = Factory::obtainPlugin (depName);

            if (depComparison == "==") {
                if (! (depVersion.toDouble() == plgn->version())) {
                    qDebug() << "(core) [AbstractPlugin] " << this->name()
                             << "requires" << depName
                             << "to have a version of" << depVersion;
                    return false;
                }
            }
            else if (depComparison == ">") {
                if (! (depVersion.toDouble () > plgn->version())) {
                    qDebug() << "(core) [AbstractPlugin] " << this->name ()
                             << "requires" << depName
                             << "to have a version greater than" << depVersion;
                    return false;
                }
            }
            else if (depComparison == "<") {
                if (! (depVersion.toDouble () < plgn->version ())) {
                    qDebug() << "(core) [AbstractPlugin] " << this->name()
                             << "requires" << depName << "to have a version less than" << depVersion;
                    return false;
                }
            }
            else if (depComparison == ">=") {
                if (! (depVersion.toDouble () >= plgn->version ())) {
                    qDebug() << "(core) [AbstractPlugin] " << this->name()
                             << "requires" << depName
                             << "to have a version of at least" << depVersion;
                    return false;
                }
            }
            else if (depComparison == "==") {
                if (! (depVersion.toDouble () > plgn->version ())) {
                    qDebug() << "(core) [AbstractPlugin] " << this->name()
                             << "requires" << depName << "to have a version of at most" << depVersion;
                    return false;
                }
            }
            else {
                qDebug() << "(core) [AbstractPlugin] <" << this->name()
                         << "> : Invalid version string ("
                         << depComparison << ").";
            }
        }
    }

    return true;
}

QStringList AbstractPlugin::packages() const
{
    Q_D (const AbstractPlugin);
    QStringList dep = d->m_sttngs->value ("Depends/Packages").toStringList();
    dep.removeDuplicates();
    dep.removeAll ("None");
    return dep;
}

/// @note This method requires code from QPackageKit.
/// @note issue #0000029
bool AbstractPlugin::hasPackages() const
{
    const QStringList deps = this->packages();

    foreach (const QString dep, deps) {
        const QString depName = dep.split (" ").at (0);
        const QString depComparison = dep.split (" ").at (1);
        const QString depVersion = dep.split (" ").at (2);
    }

    return true;
}

QVariant AbstractPlugin::attribute (const QString& p_attributePath) const
{
    Q_D (const AbstractPlugin);
    QVariant val = d->m_cnfg->value (p_attributePath);

    if (val.isNull() || !val.isValid())
        val = d->m_cnfg->value ("Configuration/" + QString (p_attributePath).replace ("/", ":"));

    return val;
}

void AbstractPlugin::setAttribute (const QString& p_attributePath, const QVariant& p_attributeValue)
{
    Q_D (AbstractPlugin);
    d->m_cnfg->setValue (p_attributePath, p_attributeValue);
}

/// @note issue #0000030
void AbstractPlugin::resetAttributes()
{
    Q_D (AbstractPlugin);
    d->m_cnfg->clear();
}

bool AbstractPlugin::loadLibrary() const
{
    Q_D (const AbstractPlugin);
    QApplication::addLibraryPath (WNTR_PLUGIN_PATH);
    const QString plgnLibrary = d->m_sttngs->value ("Version/Library").toString();
    const QString plgPth = QString (WNTR_PLUGIN_PATH) + "/lib" + plgnLibrary + ".so";
    d->m_plgnLdr = new QPluginLoader (plgPth, Factory::instance());
    d->m_plgnLdr->setLoadHints (QLibrary::ResolveAllSymbolsHint);
    d->m_plgnLdr->load();

    if (!d->m_plgnLdr->isLoaded())
        qDebug() << "(plugin) [AbstractPlugin] Error loading library"
                 << plgPth << ":" << d->m_plgnLdr->errorString();

    return d->m_plgnLdr->isLoaded();
}

bool AbstractPlugin::loadRequiredComponents() const
{
    Q_D (const AbstractPlugin);

    if (!isSupported ()) {
        qWarning() << "(plugin) [Factory] Plug-in" << name () << "is incompatible with this version of Wintermute.";
        return false;
    }
    else
        qDebug() << "(plugin) [Factory] Plug-in" << name () << "v." << version() << "is compatible with this version of Wintermute.";

    if (!d->loadPackages()) {
        qWarning() << "(plugin) [Factory] Can't load dependency packages for plug-in" << name() << ".";
        return false;
    }

    if (!d->loadPlugins()) {
        qWarning() << "(plugin) [Factory] Can't load dependency plug-ins for plug-in" << name() << ".";
        return false;
    }

    return true;
}

AbstractPlugin* AbstractPlugin::obtainInstance() const
{
    Q_D (const AbstractPlugin);

    if (d->m_plgnLdr->isLoaded())
        return qobject_cast< AbstractPlugin* > (d->m_plgnLdr->instance());

    return 0;
}

AbstractPlugin::~AbstractPlugin()
{
    Q_D (AbstractPlugin);
    d->m_plgnLdr->unload();
    d->m_plgnLdr->deleteLater();
    d->m_sttngs->deleteLater();
}

#include "plugin.hpp.moc"
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;