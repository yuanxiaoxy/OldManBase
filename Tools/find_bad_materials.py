import unreal

def find_and_highlight_bad_materials():
    """
    检查当前关卡中所有Actor的材质编译状态，找出有编译错误的材质，
    并高亮所有使用这些材质的Actor。
    """
    unreal.log("开始扫描场景中的材质编译错误...")

    # 获取编辑器世界
    editor_world = unreal.UnrealEditorSubsystem().get_editor_world()
    if not editor_world:
        unreal.log_error("无法获取编辑器世界")
        return

    # 收集有问题的材质路径和使用它们的Actor
    bad_material_paths = set()
    actors_with_bad_mats = []

    # 遍历所有Actor
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(editor_world, unreal.Actor)
    for actor in all_actors:
        # 获取所有网格组件
        components = actor.get_components_by_class(unreal.MeshComponent)
        actor_has_bad = False

        for comp in components:
            num_mats = comp.get_num_materials()
            for slot_idx in range(num_mats):
                mat_interface = comp.get_material(slot_idx)
                if not mat_interface:
                    if not actor_has_bad:
                        actors_with_bad_mats.append(actor)
                        actor_has_bad = True
                        unreal.log_warning(f"空材质槽: {actor.get_name()} 的组件 {comp.get_name()} 第{slot_idx+1}槽")
                    continue

                # 获取基础材质
                base_mat = mat_interface.get_base_material()
                if not base_mat:
                    continue

                # 关键：检查材质是否有编译错误（这是最稳定和可靠的检查方式）
                if base_mat.is_compiling_or_had_compile_error():
                    mat_path = base_mat.get_path_name()
                    if mat_path not in bad_material_paths:
                        bad_material_paths.add(mat_path)
                        unreal.log_error(f"材质编译错误: {base_mat.get_name()}")
                    if not actor_has_bad:
                        actors_with_bad_mats.append(actor)
                        actor_has_bad = True

    # 高亮并选中所有问题Actor
    if actors_with_bad_mats:
        # 使用最新的 Editor Actor Subsystem 避免弃用警告
        actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        actor_subsystem.set_selected_level_actors(actors_with_bad_mats)
        unreal.log(f"✅ 检查完成！发现 {len(bad_material_paths)} 个编译错误的材质，影响了 {len(actors_with_bad_mats)} 个Actor，已高亮选中。")
        for actor in actors_with_bad_mats:
            unreal.log(f"  问题Actor: {actor.get_name()}")
    else:
        unreal.log("✅ 检查完成！未发现材质编译错误或空材质槽。")

if __name__ == "__main__":
    find_and_highlight_bad_materials()