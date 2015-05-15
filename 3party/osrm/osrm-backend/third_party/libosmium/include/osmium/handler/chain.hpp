#ifndef OSMIUM_HANDLER_CHAIN_HPP
#define OSMIUM_HANDLER_CHAIN_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2015 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <tuple>

#include <osmium/handler.hpp>

#define OSMIUM_CHAIN_HANDLER_CALL(_func_, _type_) \
    template <int N, int SIZE, class THandlers> \
    struct call_ ## _func_ { \
        void operator()(THandlers& handlers, osmium::_type_& object) { \
            std::get<N>(handlers)._func_(object); \
            call_ ## _func_<N+1, SIZE, THandlers>()(handlers, object); \
        } \
    }; \
    template <int SIZE, class THandlers> \
    struct call_ ## _func_<SIZE, SIZE, THandlers> { \
        void operator()(THandlers&, osmium::_type_&) {} \
    };

namespace osmium {

    class Node;
    class Way;
    class Relation;
    class Area;
    class Changeset;

    namespace handler {

        /**
         * This handler allows chaining of any number of handlers into a single
         * handler.
         */
        template <class ...THandler>
        class ChainHandler : public osmium::handler::Handler {

            typedef std::tuple<THandler&...> handlers_type;
            handlers_type m_handlers;

            template <int N, int SIZE, class THandlers>
            struct call_flush {
                void operator()(THandlers& handlers) {
                    std::get<N>(handlers).flush();
                    call_flush<N+1, SIZE, THandlers>()(handlers);
                }
            }; // struct call_flush

            template <int SIZE, class THandlers>
            struct call_flush<SIZE, SIZE, THandlers> {
                void operator()(THandlers&) {}
            }; // struct call_flush

            OSMIUM_CHAIN_HANDLER_CALL(node, Node)
            OSMIUM_CHAIN_HANDLER_CALL(way, Way)
            OSMIUM_CHAIN_HANDLER_CALL(relation, Relation)
            OSMIUM_CHAIN_HANDLER_CALL(changeset, Changeset)
            OSMIUM_CHAIN_HANDLER_CALL(area, Area)

        public:

            explicit ChainHandler(THandler&... handlers) :
                m_handlers(handlers...) {
            }

            void node(osmium::Node& node) {
                call_node<0, sizeof...(THandler), handlers_type>()(m_handlers, node);
            }

            void way(osmium::Way& way) {
                call_way<0, sizeof...(THandler), handlers_type>()(m_handlers, way);
            }

            void relation(osmium::Relation& relation) {
                call_relation<0, sizeof...(THandler), handlers_type>()(m_handlers, relation);
            }

            void changeset( osmium::Changeset& changeset) {
                call_changeset<0, sizeof...(THandler), handlers_type>()(m_handlers, changeset);
            }

            void area(osmium::Area& area) {
                call_area<0, sizeof...(THandler), handlers_type>()(m_handlers, area);
            }

            void flush() {
                call_flush<0, sizeof...(THandler), handlers_type>()(m_handlers);
            }

        }; // class ChainHandler

    } // namespace handler

} // namespace osmium

#endif // OSMIUM_HANDLER_CHAIN_HPP