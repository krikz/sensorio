import os
from jsmin import jsmin
from SCons.Script import Import

# Импортируем переменную окружения env из SCons
Import("env")

def minify_js_file(input_file):
    """
    Минимизирует JS-файл и сохраняет его с суффиксом .min.js.
    :param input_file: Путь к исходному JS-файлу.
    """
    # Чтение содержимого файла
    try:
        with open(input_file, 'r', encoding='utf-8') as file:
            js_content = file.read()
    except Exception as e:
        print(f"Ошибка при чтении файла {input_file}: {e}")
        return

    # Минимизация содержимого
    try:
        minified_content = jsmin(js_content)
    except Exception as e:
        print(f"Ошибка при минимизации файла {input_file}: {e}")
        return

    # Создание имени выходного файла
    base_name, ext = os.path.splitext(input_file)
    output_file = f"{base_name}.min{ext}"

    # Запись минимизированного содержимого в новый файл
    try:
        with open(output_file, 'w', encoding='utf-8') as file:
            file.write(minified_content)
        print(f"Минимизированный файл сохранен: {output_file}")
    except Exception as e:
        print(f"Ошибка при записи файла {output_file}: {e}")

def process_specific_files(resource_dir, files_to_minify):
    """
    Обрабатывает конкретные JS-файлы, указанные в списке.
    :param resource_dir: Директория с исходными ресурсами.
    :param files_to_minify: Список файлов для минимизации (без пути).
    """
    for file in files_to_minify:
        input_file = os.path.join(resource_dir, file)
        if os.path.exists(input_file):
            minify_js_file(input_file)
        else:
            print(f"Файл не найден: {input_file}")

def before_build_action(env):
    """
    Действие, выполняемое перед сборкой проекта.
    """
    # Определяем директорию проекта из переменной окружения env
    project_dir = env['PROJECT_DIR']
    resource_dir = os.path.join(project_dir, "html")  # Директория с JS-файлами

    # Список файлов для минимизации (добавьте свои файлы)
    files_to_minify = [
        "OrbitControls.js"
    ]

    print(f"Обработка указанных JS-файлов в директории: {resource_dir}")
    process_specific_files(resource_dir, files_to_minify)

# Выполняем действие перед сборкой
before_build_action(env)