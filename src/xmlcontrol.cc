#include "xmlcontrol.h"

#include <QTextStream>

XmlControl::XmlControl(const QString& xmlfile)
    : filename_(xmlfile)
{
    read();
}

QString XmlControl::attributeValue(const QString &node_name, const QString& attribute)
{
    QDomNode node = findNode(document_.firstChild(), node_name);

    if (node.isElement())
    {
        QString attribute_value = node.toElement().attribute(attribute, ATTRIBUTE_ERROR_);

        if (attribute_value == ATTRIBUTE_ERROR_)
            throw XmlException(ATTRIBUTE_ERROR_);
        else
            return attribute_value;
    }
    else
        throw XmlException("XML error: Could not find the correct node.");
}

void XmlControl::setAttributeValue(const QString& node_name, const QString& attribute, const QString& value)
{
    QDomNode node = findNode(document_.firstChild(), node_name);

    if (node.isElement())
        node.toElement().setAttribute(attribute, value);
    else
        throw XmlException("XML error: Could not find the correct node.");

    write();
}

QString XmlControl::text(const QString& parent_node_name)
{
    QDomNode node = findNode(document_.firstChild(), parent_node_name).firstChild();

    while (!node.isNull())
    {
        if (node.isText())
            return node.toText().data();

        node = node.nextSibling();
    }

    throw XmlException("XML error: Could not find the correct node.");
}

void XmlControl::setText(const QString& parent_node_name, const QString& text)
{
    QDomNode node = findNode(document_.firstChild(), parent_node_name).firstChild();

    while (!node.isNull())
    {
        if (node.isText())
        {
            node.toText().setData(text);
            write();
            return;
        }

        node = node.nextSibling();
    }

    throw XmlException("XML error: Could not find the correct node.");
}

/*
 *  Private
 */

void XmlControl::write()
{
    QFile file(filename_);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        throw XmlException("XML error: Could not open file for writing.");
    }

    QTextStream stream(&file);

    stream << document_.toString();

    file.close();
}

void XmlControl::read()
{
    QFile file(filename_);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        throw XmlException("XML error: Could not open the file for reading.");
    }

    if (!document_.setContent(&file))
    {
        file.close();
        throw XmlException("XML error: The file is not in a valid XML format.");
    }

    file.close();
}

QDomNode XmlControl::findNode(QDomNode node, const QString& node_name)
{
    while (!node.isNull())
    {
        // Sök igenom syskon

        if (node.nodeName() == node_name)
        {
            // Returnera om rätt nod har hittats
            return node;
        }

        // Sök igenom barn

        QDomNode child = findNode(node.firstChild(), node_name);

        if (!child.isNull())
            // Rätt nod hittades som ett barn
            return child;

        // Gå till nästa syskon
        node = node.nextSibling();
    }

    return QDomNode();
}
