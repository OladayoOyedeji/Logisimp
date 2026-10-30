/**
 * @file ProjectEdit.h
 * @brief Internal rollback storage for a coordinated structural edit.
 */

#ifndef PROJECT_EDIT_H
#define PROJECT_EDIT_H

#include "Project.h"

#include <unordered_set>

/**
 * @brief Retains the old graph until replacement runtime storage is ready.
 *
 * Preparing the snapshot can throw without changing the project. Once an edit
 * starts, removed original objects remain alive so both rollback and runtime
 * migration can safely inspect them. Commit and rollback do not allocate.
 * This helper is internal to ProjectController; simulation must be paused.
 * Candidate StateUpdate objects must be destroyed before an uncommitted edit
 * rolls back, because newly created runtime states still borrow the candidate
 * structure. On success, runtime updates must commit before commit() releases
 * obsolete structural objects.
 */
class ProjectEdit
{
public:
    /**
     * @brief Snapshots the project before enabling controlled structural edits.
     * @param project Project whose original objects and ownership lists to retain.
     * @throws std::logic_error If another structural edit is already active.
     * @note Allocation failures leave the project unchanged and editing disabled.
     */
    explicit ProjectEdit(Project & project);

    /**
     * @brief Prevents copying the unique owner of rollback storage.
     * @param other Edit that cannot be copied.
     */
    ProjectEdit(const ProjectEdit & other) = delete;

    /**
     * @brief Prevents replacing a transaction through copy assignment.
     * @param other Edit that cannot be assigned.
     * @return Assignment is deleted; no reference is returned.
     */
    ProjectEdit & operator=(const ProjectEdit & other) = delete;

    /**
     * @brief Rolls back an unfinished edit and releases its snapshot storage.
     * @pre Any candidate StateUpdate objects have already been destroyed.
     * @note A committed edit is not rolled back.
     */
    ~ProjectEdit();

    /**
     * @brief Checks whether an address identifies an original structural object.
     * @param object Borrowed address to compare; it is not dereferenced.
     * @return True if the object was present when the snapshot was prepared.
     * @note Retained objects must survive candidate removal until commit or rollback.
     */
    bool retains(const void * object) const noexcept;

    /**
     * @brief Keeps the edited structure and deletes obsolete original objects.
     * @pre Runtime updates have committed and pending work no longer borrows
     * objects removed by this edit.
     * @note Does not allocate. Repeated calls after completion do nothing.
     */
    void commit() noexcept;

private:
    /** @brief Saves a circuit's ownership lists, interface order, and ID counters. */
    struct CircuitSnapshot
    {
        /**
         * @brief Copies the structural lists and counters needed for rollback.
         * @param circuit Circuit whose storage is being snapshotted.
         */
        explicit CircuitSnapshot(Circuit & circuit);

        /// Borrowed original circuit, kept alive until the transaction finishes.
        Circuit * circuit;

        /// Original owning component list, restored on rollback.
        std::vector< Component * > components;

        /// Original owning wire-node list, restored on rollback.
        std::vector< WireNode * > nodes;

        /// Original owning wire-edge list, restored on rollback.
        std::vector< WireEdge * > edges;

        /// Original owning derived-net list, restored on rollback.
        std::vector< Net * > nets;

        /// Borrowed input pins in their original interface order.
        std::vector< Pin * > inputs;

        /// Borrowed output pins in their original interface order.
        std::vector< Pin * > outputs;

        /// Next component ID before the candidate edit.
        int next_component_id;

        /// Next wire-node ID before the candidate edit.
        int next_node_id;

        /// Next wire-edge ID before the candidate edit.
        int next_edge_id;

        /// Next derived-net ID before the candidate edit.
        int next_net_id;
    };

    /** @brief Saves a component's ports, metadata, and optional pin configuration. */
    struct ComponentSnapshot
    {
        /**
         * @brief Copies mutable component data and any interface-pin shape.
         * @param component Original component to restore if the edit fails.
         */
        explicit ComponentSnapshot(Component & component);

        /// Borrowed original component retained by the transaction.
        Component * component;

        /// Original component label.
        std::string label;

        /// Original owning input-port list.
        std::vector< Port * > inputs;

        /// Original owning output-port list.
        std::vector< Port * > outputs;

        /// Next port ID before the candidate edit.
        int next_port_id;

        /// Borrowed pin view of the component, or nullptr for other component types.
        Pin * pin;

        /// Original pin role; ignored when pin is nullptr.
        PinType pin_type;

        /// Original pin signal width; ignored when pin is nullptr.
        int pin_width;
    };

    /** @brief Saves a port's metadata and borrowed connectivity pointers. */
    struct PortSnapshot
    {
        /**
         * @brief Copies the data needed to restore an original port in place.
         * @param port Original port to snapshot without changing its identity.
         */
        explicit PortSnapshot(Port & port);

        /// Borrowed original port retained by the transaction.
        Port * port;

        /// Original positive signal width.
        int width;

        /// Original input or output direction.
        PortDirection direction;

        /// Borrowed original component that owns this port.
        Component * owner;

        /// Borrowed original attachment node, or nullptr if disconnected.
        WireNode * node;

        /// Borrowed original derived net, or nullptr if no net was assigned.
        Net * net;

        /// Original port label.
        std::string label;
    };

    /** @brief Saves a wire node's metadata, adjacency, and attached ports. */
    struct NodeSnapshot
    {
        /**
         * @brief Copies the data needed to restore an original wire node.
         * @param node Original node whose connectivity and metadata to save.
         */
        explicit NodeSnapshot(WireNode & node);

        /// Borrowed original node retained by the transaction.
        WireNode * node;

        /// Original width constraint; zero means unspecified.
        int width;

        /// Original node label.
        std::string label;

        /// Borrowed incident edges in their original order.
        std::vector< WireEdge * > edges;

        /// Borrowed attached ports in their original order.
        std::vector< Port * > ports;

        /// Borrowed original derived net, or nullptr for a region without attached ports.
        Net * net;
    };

    /**
     * @brief Revokes edit permission from the project and retained objects.
     * @note Covers both original objects removed by the edit and surviving objects.
     */
    void disable() noexcept;

    /**
     * @brief Deletes candidate objects and restores original storage in place.
     * @pre Candidate runtime updates have been destroyed before graph restoration.
     * @note Restores lists and labels using swaps without allocating.
     */
    void rollback() noexcept;

    /// Borrowed project whose edit permission and structural ownership are coordinated.
    Project & project_;

    /// Original owning circuit list, retained for rollback and retirement checks.
    std::vector< Circuit * > circuits_;

    /// Original registry of borrowed parent-circuit and placement pointers.
    std::unordered_map< const Circuit *, std::vector< Project::Placement > > placements_;

    /// Snapshots of every original circuit, including circuits later removed.
    std::vector< CircuitSnapshot > circuit_snapshots_;

    /// Snapshots of every original component and its port ownership lists.
    std::vector< ComponentSnapshot > component_snapshots_;

    /// Snapshots of every original port and its attachment metadata.
    std::vector< PortSnapshot > port_snapshots_;

    /// Snapshots of every original wire node and its adjacency lists.
    std::vector< NodeSnapshot > node_snapshots_;

    /// Non-owning addresses of original objects whose deletion must be deferred.
    std::unordered_set< const void * > retained_;

    /// True once commit or rollback has completed and editing is disabled.
    bool finished_;
};

#endif // PROJECT_EDIT_H
